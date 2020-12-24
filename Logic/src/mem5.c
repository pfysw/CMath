/*
** 2007 October 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement a memory
** allocation subsystem for use by SQLite.
**
** This version of the memory allocation subsystem omits all
** use of malloc(). The application gives SQLite a block of memory
** before calling sqlite3_initialize() from which allocations
** are made and returned by the xMalloc() and xRealloc()
** implementations. Once sqlite3_initialize() has been called,
** the amount of memory available to SQLite is fixed and cannot
** be changed.
**
** This version of the memory allocation subsystem is included
** in the build only if SQLITE_ENABLE_MEMSYS5 is defined.
**
** This memory allocator uses the following algorithm:
**
**   1.  All memory allocation sizes are rounded up to a power of 2.
**
**   2.  If two adjacent free blocks are the halves of a larger block,
**       then the two blocks are coalesced into the single larger block.
**
**   3.  New memory is allocated from the first available free block.
**
** This algorithm is described in: J. M. Robson. "Bounds for Some Functions
** Concerning Dynamic Storage Allocation". Journal of the Association for
** Computing Machinery, Volume 21, Number 8, July 1974, pages 491-499.
**
** Let n be the size of the largest allocation divided by the minimum
** allocation size (after rounding all sizes up to a power of 2.)  Let M
** be the maximum amount of memory ever outstanding at one time.  Let
** N be the total amount of memory available for allocation.  Robson
** proved that this memory allocator will never breakdown due to
** fragmentation as long as the following constraint holds:
**
**      N >=  M*(1 + log2(n)/2) - n + 1
**
** The sqlite3_status() logic tracks the maximum values of n and M so
** that an application can, at any time, verify this constraint.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "token.h"
#include "mem5.h"


/*
** Unlink the chunk at pMem->aPool[i] from list it is currently
** on.  It should be found on pMem->aiFreelist[iLogsize].
*/
static void memsys5Unlink(Mem5Global *pMem, int i, int iLogsize){
  int next, prev;
  assert( i>=0 && i<pMem->nBlock );
  assert( iLogsize>=0 && iLogsize<=LOGMAX );
  assert( (pMem->aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  next = MEM5LINK(i)->next;
  prev = MEM5LINK(i)->prev;
  if( prev<0 ){
    pMem->aiFreelist[iLogsize] = next;
  }else{
    MEM5LINK(prev)->next = next;
  }
  if( next>=0 ){
    MEM5LINK(next)->prev = prev;
  }
}

/*
** Link the chunk at pMem->aPool[i] so that is on the iLogsize
** free list.
*/
static void memsys5Link(Mem5Global *pMem, int i, int iLogsize){
  int x;
  assert( i>=0 && i<pMem->nBlock );
  assert( iLogsize>=0 && iLogsize<=LOGMAX );
  assert( (pMem->aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->next = pMem->aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  if( x>=0 ){
    assert( x<pMem->nBlock );
    MEM5LINK(x)->prev = i;
  }
  pMem->aiFreelist[iLogsize] = i;
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
int memsys5Size(Mem5Global *pMem, void *p){
  int iSize, i;
  assert( p!=0 );
  i = (int)(((u8 *)p-pMem->zPool)/pMem->szAtom);
  assert( i>=0 && i<pMem->nBlock );
  iSize = pMem->szAtom * (1 << (pMem->aCtrl[i]&CTRL_LOGSIZE));
  return iSize;
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.  Return NULL if nBytes==0.
**
** The caller guarantees that nByte is positive.
**
** The caller has obtained a mutex prior to invoking this
** routine so there is never any chance that two or more
** threads can be in this routine at the same time.
*/
void *memsys5Malloc(Mem5Global *pMem, int nByte){
  int i;           /* Index of a pMem->aPool[] slot */
  int iBin;        /* Index into pMem->aiFreelist[] */
  int iFullSz;     /* Size of allocation rounded up to power of 2 */
  int iLogsize;    /* Log2 of iFullSz/POW2_MIN */


  /* nByte must be a positive */
  assert( nByte>0 );

  /* No more than 1GiB per allocation */
  if( nByte > 0x40000000 ) return 0;


  /* Round nByte up to the next valid power of two */
  for(iFullSz=pMem->szAtom,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

  /* Make sure pMem->aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  for(iBin=iLogsize; iBin<=LOGMAX && pMem->aiFreelist[iBin]<0; iBin++){}
  if( iBin>LOGMAX ){
    printf("no mem %d\n",nByte);
    sleep(1);
    assert(0);
  }
  i = pMem->aiFreelist[iBin];
  memsys5Unlink(pMem, i, iBin);
  while( iBin>iLogsize ){
    int newSize;

    iBin--;
    newSize = 1 << iBin;
    pMem->aCtrl[i+newSize] = CTRL_FREE | iBin;
    memsys5Link(pMem, i+newSize, iBin);
  }
  pMem->aCtrl[i] = iLogsize;


  /* Return a pointer to the allocated memory. */
  return (void*)&pMem->zPool[i*pMem->szAtom];
}

/*
** Free an outstanding memory allocation.
*/
void memsys5Free(Mem5Global *pMem, void *pOld){
  u32 size, iLogsize;
  int iBlock;


  /* Set iBlock to the index of the block pointed to by pOld in
  ** the array of pMem->szAtom byte blocks pointed to by pMem->zPool.
  */
  iBlock = (int)(((u8 *)pOld-pMem->zPool)/pMem->szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  assert( iBlock>=0 && iBlock<pMem->nBlock );
  assert( ((u8 *)pOld-pMem->zPool)%pMem->szAtom==0 );
  assert( (pMem->aCtrl[iBlock] & CTRL_FREE)==0 );

  iLogsize = pMem->aCtrl[iBlock] & CTRL_LOGSIZE;
  size = 1<<iLogsize;
  assert( iBlock+size-1<(u32)pMem->nBlock );

  pMem->aCtrl[iBlock] |= CTRL_FREE;
  pMem->aCtrl[iBlock+size-1] |= CTRL_FREE;

  pMem->aCtrl[iBlock] = CTRL_FREE | iLogsize;
  while( ALWAYS(iLogsize<LOGMAX) ){
    int iBuddy;
    if( (iBlock>>iLogsize) & 1 ){
      iBuddy = iBlock - size;
      assert( iBuddy>=0 );
    }else{
      iBuddy = iBlock + size;
      if( iBuddy>=pMem->nBlock ) break;
    }
    if( pMem->aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) break;
    memsys5Unlink(pMem, iBuddy, iLogsize);
    iLogsize++;
    if( iBuddy<iBlock ){
      pMem->aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      pMem->aCtrl[iBlock] = 0;
      iBlock = iBuddy;
    }else{
      pMem->aCtrl[iBlock] = CTRL_FREE | iLogsize;
      pMem->aCtrl[iBuddy] = 0;
    }
    size *= 2;
  }


  //log_a("free %d",pJunqi->free_cnt);
  memsys5Link(pMem, iBlock, iLogsize);
}

/*
** Round up a request size to the next valid allocation size.  If
** the allocation is too large to be handled by this allocation system,
** return 0.
**
** All allocations must be a power of two and must be expressed by a
** 32-bit signed integer.  Hence the largest allocation is 0x40000000
** or 1073741824 bytes.
*/
int memsys5Roundup(Mem5Global *pMem, int n){
  int iFullSz;
  if( n > 0x40000000 ) return 0;
  for(iFullSz=pMem->szAtom; iFullSz<n; iFullSz *= 2);
  return iFullSz;
}

/*
** Return the ceiling of the logarithm base 2 of iValue.
**
** Examples:   memsys5Log(1) -> 0
**             memsys5Log(2) -> 1
**             memsys5Log(4) -> 2
**             memsys5Log(5) -> 3
**             memsys5Log(8) -> 3
**             memsys5Log(9) -> 4
*/
static int memsys5Log(int iValue){
  int iLog;
  for(iLog=0; (iLog<(int)((sizeof(int)*8)-1)) && (1<<iLog)<iValue; iLog++);
  return iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
Mem5Global *memsys5Init(int nHeap, int mnReq){
  int ii;            /* Loop counter */
  int nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  int nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  int iOffset;       /* An offset into pMem->aCtrl[] */


  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  assert( (sizeof(Mem5Link)&(sizeof(Mem5Link)-1))==0 );
  Mem5Global *pMem = NULL;
  pMem = (Mem5Global*)malloc(sizeof(Mem5Global)+nHeap);
  memset(pMem,0,sizeof(Mem5Global));

  nByte = nHeap;
  zByte = (u8*)&pMem[1];
  assert( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(mnReq);
  pMem->szAtom = (1<<nMinLog);
  while( (int)sizeof(Mem5Link)>pMem->szAtom ){
    pMem->szAtom = pMem->szAtom << 1;
  }

  pMem->nBlock = (nByte / (pMem->szAtom+sizeof(u8)));
  pMem->zPool = zByte;
  pMem->aCtrl = (u8 *)&pMem->zPool[pMem->nBlock*pMem->szAtom];

  for(ii=0; ii<=LOGMAX; ii++){
    pMem->aiFreelist[ii] = -1;
  }

  iOffset = 0;
  for(ii=LOGMAX; ii>=0; ii--){
    int nAlloc = (1<<ii);
    if( (iOffset+nAlloc)<=pMem->nBlock ){
      pMem->aCtrl[iOffset] = ii | CTRL_FREE;
      memsys5Link(pMem, iOffset, ii);
      iOffset += nAlloc;
    }
    assert((iOffset+nAlloc)>pMem->nBlock);
  }


  return pMem;
}

/*
** Deinitialize this module.
*/
void memsys5Shutdown(Mem5Global **pMem){
  free(*pMem);
  *pMem = NULL;
}
