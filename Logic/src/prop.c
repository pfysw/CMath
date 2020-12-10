/*
 * prop.c
 *
 *  Created on: Aug 29, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include <assert.h>
#include "prop.h"

typedef struct PermData
{
    u8 nAll;
    u8 iNofree;
    u8 nFree;
    u8 pos;
    u8 aMap[10];
}PermData;

typedef struct RefTherom
{
    int nRef;
    int aSet[100];
}RefTherom;

RefTherom refset;

typedef struct RefTable
{
    int refprop;
    int reftime;
}RefTable;

RefTable aRefTable[100] =
{
    {0,0},
    {1,1},
    {2,2},
    {22,22},//  (A->B)->((C->A)->(C->B)))
    {39,39},  //(((A->B)->(C->A))->((A->B)->(C->B)))
    {124,124},//(~~A->(B->A))
    {129,129},//  ~~A->A
    {186,186},// (A->(~~B->B))
    {341,341},  //((A->B)->(~~A->B))
    {322,500},  //(A->(B->~~B))    9
    //((A->B)->(A->~~B))
    {677,1000}, //((A->B)->(~B->~A))
    {121,677},   //(~A->(A->B))     11
    {1921,1921},     //((~A->A)->(~A->B))
    {2174,2174},  //(((~A->A)->~B)->((~A->A)->~~A))
    {188,188},    //(A->~~A)   14
    //num 2728 : ((~A->A)->~~A)
    //num 2731 : ((~A->A)->A)
};


#define REF_NUM    15
#define REF_TEST    3000

u8 isEqualNode(TokenInfo *pA,TokenInfo *pB)
{
    u8 rc = 0;
    if( pA->symb == pB->symb )
    {
        rc = 1;
    }

    return rc;
}

int SetSameNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    static int n = 0;
    static int cnt = 0;
    int i;
    int rc;

    if( !cnt )
    {
        n = 0;
    }
    cnt++;
    if(  (*ppAst)->type==PROP_SYMB )
    {
        rc = 0;
        for( i=0;i<n;i++ )
        {
            if( isEqualNode((*ppAst),ppTemp[i]) )
            {
                if(*ppAst!=ppTemp[i])
                {
                    FreeAstNode(pParse,(*ppAst));
                    *ppAst = ppTemp[i];
                }
                rc = 1;
                break;
            }
        }
        if( !rc )
        {
            ppTemp[n++] = *ppAst;
        }
    }
    else
    {
        SetSameNode(pParse,&((*ppAst)->pLeft),ppTemp);
        if( (*ppAst)->type==PROP_IMPL )
            SetSameNode(pParse,&((*ppAst)->pRight),ppTemp);
    }
    cnt--;
    return n;
}

int GetDiffNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp,
        u8 isSubst)
{
    static int n = 0;
    static int cnt = 0;
    int i;
    int rc;
    TokenInfo *p;

    if( !cnt )
    {
        n = 0;
    }
    cnt++;
    if( cnt>10 )
    {
        pParse->bDiscard = 1;
    }
    if(  (*ppAst)->type==PROP_SYMB )
    {

        if( (*ppAst)->bSubst && isSubst &&
                ((*ppAst)->pSubst->type!=PROP_SYMB  ||
                        (*ppAst)->pSubst->bSubst ) )
        {
            cnt--;
            GetDiffNode(pParse,&((*ppAst)->pSubst),ppTemp,isSubst);
            cnt++;
        }
        else
        {

            rc = 0;
            if( (*ppAst)->bSubst && isSubst )
            {
                p = (*ppAst)->pSubst;
            }
            else
            {
                p = (*ppAst);
            }

            for( i=0;i<n;i++ )
            {
                if( p == ppTemp[i] )
                {
                    rc = 1;
                    break;
                }
            }
            if( !rc )
            {
                ppTemp[n++] = p;
            }

        }

    }
    else
    {
        GetDiffNode(pParse,&((*ppAst)->pLeft),ppTemp,isSubst);
        if( (*ppAst)->type==PROP_IMPL )
            GetDiffNode(pParse,&((*ppAst)->pRight),ppTemp,isSubst);
    }
    cnt--;
    return n;
}

int GetAllNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    static int n = 0;
    static int cnt = 0;

    if( !cnt )
    {
        n = 0;
    }
    cnt++;
    if(  (*ppAst)->type==PROP_SYMB )
    {

        if( (*ppAst)->bSubst && (*ppAst)->pSubst->type!=PROP_SYMB )
        {
            GetAllNode(pParse,&((*ppAst)->pSubst),ppTemp);
        }
        else
        {
            n++;
        }

    }
    else
    {
        GetAllNode(pParse,&((*ppAst)->pLeft),ppTemp);
        if( (*ppAst)->type==PROP_IMPL )
            GetAllNode(pParse,&((*ppAst)->pRight),ppTemp);
    }
    cnt--;
    return n;
}



void SubstFreeTerm(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        PermData data)
{
    int i,j,t;
    PermData para;
    para.pos = data.pos+1;
    para.nAll = data.nAll;

    assert( para.pos<10 );
    if( 0==data.nFree )
    {
        PrintAst(pParse,pAst);
        return;
    }


    for(i=1; i<(1<<data.nFree); i++)
    {
        para.nFree = 0;
        if( !(i&1) )
        {
            continue;
        }
        for(j=0; j<data.nFree; j++)
        {
            if( (1<<j)&i )
            {
                t = data.aMap[j];
                ppTemp[t]->symb = 'A'+data.pos;
            }
            else
            {
                para.aMap[para.nFree] = data.aMap[j];
                para.nFree++;
            }
        }
        SubstFreeTerm(pParse,pAst,ppTemp,para);
    }
}

void SubstNofreeTerm(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        PermData data)
{
    int i,j;

    assert( data.pos>0 );
    if( (1<<data.nAll)-1==data.iNofree )
    {
        SubstFreeTerm(pParse,pAst,ppTemp,data);
        return;
    }

    for(j=0; j<data.nAll; j++)
    {
        if( !((1<<j)&data.iNofree) )//找到一个非自由变元的位置
        {
            data.iNofree |= (1<<j);
            for(i=0; i<data.pos; i++)
            {
                ppTemp[j]->symb = 'A'+i;//在这个位置上遍历所有非自由变元
                SubstNofreeTerm(pParse,pAst,ppTemp,data);
            }
            break;
        }
    }
}

void PermPropSymb(
        AstParse *pParse,
        TokenInfo *pAst,
        TokenInfo **ppTemp,
        int nAll,
        int pos)
{
    int i,j;
    PermData data;

    memset(&data,0,sizeof(data));
    data.nAll = nAll;
    data.pos = pos;
    if( pos>0 )
    {
        for(i=0; i<(1<<nAll); i++)
        {
            memset(data.aMap,0,sizeof(data.aMap));
            data.nFree = 0;
            data.iNofree = i;
            for(j=0; j<nAll; j++)
            {
                if( (1<<j)&i )
                {
                    data.aMap[data.nFree] = j;
                    data.nFree++;
                }
            }
            SubstNofreeTerm(pParse,pAst,ppTemp,data);
        }
    }
    else
    {
        memset(data.aMap,0,sizeof(data.aMap));
        data.nFree = 0;
        for(j=0; j<nAll; j++)
        {
            data.aMap[data.nFree] = j;
            data.nFree++;
        }
        SubstFreeTerm(pParse,pAst,ppTemp,data);
    }
}

void GenBasicProp(AstParse *pParse)
{
    TokenInfo *pNode[5];
    TokenInfo *ppTemp[10];
    int i;
    int n;

    for(i=0;i<5;i++)
    {
        pNode[i] = NewNode(pParse);
        pNode[i]->symb = 'A'+i;
        pNode[i]->type = PROP_SYMB;
        //log_a("i %d %s",i,pNode[i]->zSymb);
        PrintAst(pParse,pNode[i]);
    }
    log_a("test1");
    PrintAst(pParse,pParse->pRoot);

    //PrintAstAddr(pParse,pParse->pRoot);
    log_a("set same");
    n = SetSameNode(pParse,&pParse->pRoot,ppTemp);
    log_a("n %d",n);
    PermPropSymb(pParse,pParse->pRoot,ppTemp,n,1);
    //PrintAstAddr(pParse,pParse->pRoot);
}

int isChildProp(AstParse *pParse,TokenInfo *pProp,TokenInfo *pSym)
{
    int rc = 0;

    assert( pSym->type==PROP_SYMB );

    if( pProp->type==PROP_SYMB )
    {
        if( pProp->bSubst && pProp->pSubst->type!=PROP_SYMB )
        {
            return isChildProp(pParse,pProp->pSubst,pSym);
        }

        if( pProp==pSym )
        {
            return 1;
        }
        else if( pProp->bSubst )
        {
            return isChildProp(pParse,pProp->pSubst,pSym);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        rc = isChildProp(pParse,pProp->pLeft,pSym);
        if( !rc && pProp->type==PROP_IMPL )
        {
            rc = isChildProp(pParse,pProp->pRight,pSym);
        }
    }
    return rc;
}
//把pA表示成pB的形式
int  SubstProp(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB)
{
    int rc = 0;

    while( pA->bSubst )
    //if( pA->bSubst && pA->pSubst->type!=PROP_SYMB )
    {
        pA = pA->pSubst;
    }
    while( pB->bSubst )
    //if( pB->bSubst && pB->pSubst->type!=PROP_SYMB )
    {
        pB = pB->pSubst;
    }

    if( pA->type==PROP_SYMB )
    {
        if( pB->type==PROP_SYMB )
        {
            //if( !pA->bSubst && pA!=pB )
            if( !pA->bSubst && !isChildProp(pParse,pB,pA) )
            {
                pA->pSubst = pB;
                pA->bSubst = 1;
            }
//            if( !pB->bSubst )
//            {
//                pB->pSubst = pA;
//                pB->bSubst = 1;
//            }

            return 1;
        }
        else
        {
            if( isChildProp(pParse,pB,pA) )
            {
                return 0;
            }
            else
            {
                pA->pSubst = pB;
                pA->bSubst = 1;
                rc = 1;
            }
        }
    }
    else
    {
        if( pB->type==PROP_SYMB )
        {
            if( isChildProp(pParse,pA,pB) )
            {
                return 0;
            }
            else
            {
                pB->pSubst = pA;
                pB->bSubst = 1;
                rc = 1;
            }
        }
        else if( pB->type!=pA->type )
        {
            return 0;
        }
        else
        {
            rc = SubstProp(pParse,pA->pLeft,pB->pLeft);
            if( rc&&pA->type==PROP_IMPL )
            {
                rc = SubstProp(pParse,pA->pRight,pB->pRight);
            }
        }
    }
    return rc;
}

void InsertVector(Vector *pV,TokenInfo *pData)
{

    if( pV->n==2978 )
    {
        log_a("ss");
    }
    if( pV->n<pV->size )
    {
        pV->data[pV->n++] = pData;
    }
    else
    {
        assert( pV->size<1000000 );

        pV->size = pV->size*2;
        pV->data = realloc(pV->data,sizeof(TokenInfo **)*pV->size);
        pV->data[pV->n++] = pData;
    }
}

void ClearSubstFlag(AstParse *pParse,TokenInfo *pAst)
{
    assert(pAst!=NULL);

    if( pAst->type==PROP_SYMB )
    {
        pAst->bSubst = 0;
    }
    else if( pAst->type==PROP_NEG )
    {
        pAst->bSubst = 0;
        ClearSubstFlag(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        pAst->bSubst = 0;
        ClearSubstFlag(pParse,pAst->pLeft);
        ClearSubstFlag(pParse,pAst->pRight);
    }
}

#define DEBUG 1
#define INDEX_I   75
#define INDEX_J   15
#define NUM_NOT_SAME   5
#define LOOP_N         5


Vector theoremset;
u8 aCnt[1000] = {0};

void InitTheoremSet(void)
{
    memset(&theoremset,0,sizeof(Vector));
    theoremset.size = 100;
    theoremset.data = malloc(theoremset.size*sizeof(TokenInfo **));
}

void SetRepeatFlag(int i,int j,int hs,TokenInfo *pCopy)
{
    if( hs )
    {
        if( 2==theoremset.data[j]->isRightTheorem )
        {
            pCopy->isRightTheorem = 2;
        }
    }
    else
    {
        if( 0==j )
        {
            pCopy->isRightTheorem = 1;
            pCopy->iRight = i;
        }
        else if( 0==i&&11==j )
        {
            //(A->(B->A))    0
            //((A->B)->(A->(C->B)))  11
            //(A->(B->(C->A)))
            pCopy->isRightTheorem = 2;
        }
        //((A->(B->C))->((A->B)->(A->C)))
        else if( 1==j )
        {
            int iRight;
            assert( theoremset.data[i]->type==PROP_IMPL );
            if( theoremset.data[i]->isRightTheorem )
            {
                iRight = theoremset.data[i]->iRight;
                if( theoremset.data[iRight]->isRightTheorem )
                {
                    pCopy->isRightTheorem = 1;
                }
            }
        }
        else
        {
            if( i==4 )
            {
                pCopy->isRightTheorem = 1;
            }
        }
    }
}

int ProRepeaProp(int i,int j,int hs)
{
    int rc = 0;

    if( hs )
    {
        //A->A
        if( 5==i || 5==j  )
        {
            return 1;
        }
    }
    else
    {
        if( theoremset.data[j]->isRightTheorem )
        {
            return 1;
        }
        else if( theoremset.n>5 )
        {
            if( j==5 )
            {
                return 1;
            }
            //A->A   (~A->~B)->(B->A)
            if( i==5 && j==2 )
            {
                return 1;
            }
            if( i==4 && j==1 )
            {
                return 1;
            }
            //124  ((~~A->~~B)->(A->B))
            //39 ((A->B)->(C->((D->A)->(D->B))))
//            if( i==5 && (j==22||j==39||j==124))
//            {
//                return 1;
//            }
        }
        else if( 5==theoremset.n )
        {
            theoremset.data[4]->isRightTheorem = 1;
        }
    }

    return rc;
}

int InsertMpProp(
        AstParse *pParse,
        TokenInfo **ppTemp,
        TokenInfo **apCopy,
        int i,int j)
{
    int k;
    int n,m;


    m = GetAllNode(pParse,&theoremset.data[j]->pRight,ppTemp);
#if DEBUG&&0
    log_a("m %d",m);
#endif
    if( m>10 ) return 0;

    n = GetDiffNode(pParse,&theoremset.data[j]->pRight,ppTemp,1);
#if DEBUG&&0
    log_a("n %d",n);
#endif
    if( n>NUM_NOT_SAME || pParse->bDiscard )
    {
        pParse->bDiscard = 0;
        return 0;
    }


    for(k=0; k<n; k++)
    {
        //log_a("sym %c",ppTemp[k]->symb);
        //ppTemp[k]->symb = 'A'+k;
        ppTemp[k]->copy = 'A'+k;
    }

    apCopy[1] = CopyAstTree(pParse,theoremset.data[j]->pRight,1);

    SetSameNode(pParse,&apCopy[1],ppTemp);

    SetRepeatFlag(i,j,0,apCopy[1]);

    InsertVector(&theoremset,apCopy[1]);

    n = GetDiffNode(pParse,&theoremset.data[j],ppTemp,0);
    for(k=0; k<n; k++)
    {
        //log_a("sym %c",ppTemp[k]->symb);
        ppTemp[k]->symb = 'A'+k;
    }

    return 1;
}

int InsertHSProp(
        AstParse *pParse,
        TokenInfo **ppTemp,
        TokenInfo **apCopy,
        int i,int j)
{
    int k;
    int n, m;
    int rc = 1;

    apCopy[2] = NewNode(pParse);
    apCopy[2]->type = PROP_IMPL;
    apCopy[2]->pRight = theoremset.data[j]->pRight;
    apCopy[2]->pLeft = theoremset.data[i]->pLeft;

    m = GetAllNode(pParse,&apCopy[2],ppTemp);
    if( m>10 )
    {
        rc = 0;
        goto free_node;
    }

    n = GetDiffNode(pParse,&apCopy[2],ppTemp,1);
    if( n>NUM_NOT_SAME || pParse->bDiscard )
    {
        pParse->bDiscard = 0;
        rc = 0;
        goto free_node;
    }

    for(k=0; k<n; k++)
    {
        ppTemp[k]->copy = 'A'+k;
    }

    apCopy[1] = CopyAstTree(pParse,apCopy[2],1);

    SetSameNode(pParse,&apCopy[1],ppTemp);
    InsertVector(&theoremset,apCopy[1]);

    n = GetDiffNode(pParse,&theoremset.data[j],ppTemp,0);
    pParse->bDiscard = 0;
    for(k=0; k<n; k++)
    {
        ppTemp[k]->symb = 'A'+k;
    }
    n = GetDiffNode(pParse,&theoremset.data[i],ppTemp,0);
    for(k=0; k<n; k++)
    {
        ppTemp[k]->symb = 'A'+k;
    }
    SetRepeatFlag(i,j,1,apCopy[1]);
free_node:
    FreeAstNode(pParse,apCopy[2]);
    return rc;
}

int  MpRule(
        AstParse *pParse,
        TokenInfo **ppTemp,
        int i,
        int j,
        int hs)
{
    int n;
    int rc = 0;

    TokenInfo *apCopy[5];

    if( ProRepeaProp(i,j,hs) )
    {
        return 0;
    }

    n = 0;
    if(i==j)
    {


        if( theoremset.data[i]->type!=PROP_IMPL )
        {
            goto end_insert;
        }

        apCopy[0] = CopyAstTree(pParse,theoremset.data[i],0);
        SetSameNode(pParse,&apCopy[0],ppTemp);

        if( hs )
        {
//            if( apCopy[0]->type!=PROP_IMPL )
//            {
//                goto end_insert;
//            }
            rc = SubstProp(pParse,theoremset.data[j]->pLeft,apCopy[0]->pRight);
        }
        else
        {
            rc = SubstProp(pParse,theoremset.data[j]->pLeft,apCopy[0]);
        }

    }
    else
    {
       // log_a("i %d",i);
      //  PrintAst(pParse,theoremset.data[i]);

        if( theoremset.data[j]->type!=PROP_IMPL ) goto end_insert;
        if( hs )
        {
            if( theoremset.data[i]->type!=PROP_IMPL ) goto end_insert;
            rc = SubstProp(pParse,
                            theoremset.data[j]->pLeft,theoremset.data[i]->pRight);
        }
        else
        {
            rc = SubstProp(pParse,
                    theoremset.data[j]->pLeft,theoremset.data[i]);
        }
    }
#if DEBUG
   // log_a("rc %d i %d j %d",rc ,i,j);
#endif
    if( rc )
    {

        if( hs )
        {
            if( !InsertHSProp(pParse,ppTemp,apCopy,i,j) )
            {
                goto end_insert;
            }
        }
        else
        {
            if( !InsertMpProp(pParse,ppTemp,apCopy,i,j) )
            {
                goto end_insert;
            }
        }

        //if( i>2 && j>2 )
        {
#if DEBUG
            //PrintSubstAst(pParse,theoremset.data[i]);
            log_a("hs:%d i: %d",hs,i);
            PrintAst(pParse,theoremset.data[i]);
            //PrintSubstAst(pParse,theoremset.data[i]);
            log_a("hs:%d j: %d",hs,j);
            PrintAst(pParse,theoremset.data[j]);
           // PrintSubstAst(pParse,theoremset.data[j]);
            log_a("----");

#endif
            if( n<NUM_NOT_SAME+1 )
            {
                log_c("num %d : ",theoremset.n-1);
                PrintSubstAst(pParse,apCopy[1]);
            }
        }
    }
end_insert:
    ClearSubstFlag(pParse,theoremset.data[i]);
    ClearSubstFlag(pParse,theoremset.data[j]);
    if( i==j )
    {
        FreeAstTree(pParse,&apCopy[0],ppTemp);
    }

    return rc;
}


TokenInfo *  PropMpSubst(
        AstParse *pParse,
        TokenInfo **ppTemp,
        TokenInfo *pA,//条件
        TokenInfo *pB)//定理
{
    int rc = 0;
    int k;
    TokenInfo *apCopy[5] = {0};

    SetSameNode(pParse,&pA,ppTemp);
    SetSameNode(pParse,&pB,ppTemp);
    if(pA==pB)
    {
        if( pB->type!=PROP_IMPL )
        {
            goto end_insert;//不可能出现这种情况
        }
        apCopy[0] = CopyAstTree(pParse,pB,0);
        SetSameNode(pParse,&apCopy[0],ppTemp);
        rc = SubstProp(pParse,apCopy[0],pB->pLeft);
    }
    else
    {
        rc = SubstProp(pParse,pA,pB->pLeft);
    }

    if( rc )
    {
        int n,m;

        m = GetAllNode(pParse,&pB->pRight,ppTemp);
        if( m>10 ) {
            printf("GetAllNode num %d\n",m);
            goto end_insert;
        }
        n = GetDiffNode(pParse,&pB->pRight,ppTemp,1);
        if( n>NUM_NOT_SAME || pParse->bDiscard )
        {
            printf("GetDiffNode %d bDiscard %d\n",n,pParse->bDiscard);
            pParse->bDiscard = 0;
            goto end_insert;
        }
        for(k=0; k<n; k++)
        {
            ppTemp[k]->copy = 'A'+k;
        }
        apCopy[1] = CopyAstTree(pParse,pB->pRight,1);
        SetSameNode(pParse,&apCopy[1],ppTemp);
       // PrintSubstAst(pParse,apCopy[1]);
    }
    else
    {
        log_a("MpSubst fail");
    }

end_insert:
    ClearSubstFlag(pParse,pA);
    ClearSubstFlag(pParse,pB);
    if( pA==pB )
    {
        FreeAstTree(pParse,&apCopy[0],ppTemp);
    }

    return apCopy[1];
}

void SetRefNum(int i)
{
    refset.aSet[refset.nRef++] = i;
}

int DeepSearch(
        AstParse *pParse,
        TokenInfo **ppTemp,
        int idx,
        int iBegin,
        int max)
{
    int j;
    int nNow;
    int nNow1;
    int rc;
    int temp;

    for(j=iBegin; (j<refset.nRef)&&j<theoremset.n; j++)
    {
        rc = MpRule(pParse,ppTemp,idx,refset.aSet[j],0);

        if( theoremset.n>REF_TEST )
        {
            rc = MpRule(pParse,ppTemp,refset.aSet[j],idx,0);
            continue;
        }

        temp = theoremset.n-1;;
        if( refset.nRef==13 )
        {
            log_a("test1");
            if( MpRule(pParse,ppTemp,idx,refset.aSet[j],1) )
            {
                MpRule(pParse,ppTemp,theoremset.n-1,1,0);
            }
        }
        if( refset.nRef==15 )
        {
            for(int k=0;k<refset.nRef;k++)
            {
                MpRule(pParse,ppTemp,temp,refset.aSet[k],1);
            }
        }

        if(  rc )
        {
            nNow = temp;
            //nNow = theoremset.n-1;
            for(int k=0;k<refset.nRef;k++)
            {
                int r;
                //公理做前件
                //r = MpRule(pParse,ppTemp,k,nNow,0);
                r = MpRule(pParse,ppTemp,refset.aSet[k],nNow,0);// todo 应是refset.aSet[j]
                if( refset.nRef==10 )
                {
                    MpRule(pParse,ppTemp,nNow,refset.aSet[k],1);
                    MpRule(pParse,ppTemp,refset.aSet[k],nNow,1);
                }
                if( r )
                {
                    nNow1 = theoremset.n-1;

                    for(int k=0;k<refset.nRef;k++)
                    {
                        //HS
                        MpRule(pParse,ppTemp,nNow1,refset.aSet[k],1);
                        MpRule(pParse,ppTemp,refset.aSet[k],nNow1,1);
                    }
                }
                if( r && refset.nRef<10 )
                {
                    nNow = nNow1;//保留之前的bug，如果改掉则引理表都要重写
                }

            }
        }
    }
    return j;
}

void  SubstPropTest(
        AstParse *pParse,
        TokenInfo **ppTest)
{
    int i,j,k;
    int n;
    int rc;
    TokenInfo *ppTemp[100];
    int iLoop = 0;

    memset(&refset,0,sizeof(refset));

    for(i=0; i<3; i++)
    {
        PrintAst(pParse,ppTest[i]);
        n = SetSameNode(pParse,&ppTest[i],ppTemp);
        log_a("n %d",n);
        InsertVector(&theoremset,ppTest[i]);
        PrintAst(pParse,theoremset.data[i]);
        SetRefNum(i);
    }
    log_a("***********");
    for(i=0; i<INDEX_I&&i<theoremset.n; i++)
    {
        aCnt[i] = DeepSearch(pParse,ppTemp,i,aCnt[i],INDEX_J);
        if( theoremset.n>3000 )
        {
            for(int j=3;j<refset.nRef;j++)
            {
                //refset.aSet[j]++;
                refset.aSet[j] = j+theoremset.n-3000;
            }
        }
        if( refset.nRef<REF_NUM &&
                theoremset.n<REF_TEST &&
                theoremset.n>aRefTable[refset.nRef].reftime)
        {
            if(refset.nRef==10)
            {
                log_a("*");
            }
            int iNew;
            iNew = aRefTable[refset.nRef].refprop;

            assert(iNew!=15);
            SetRefNum(iNew);
            if(iNew<1000)
            {
                aCnt[iNew] = DeepSearch(pParse,ppTemp,iNew,aCnt[iNew],refset.nRef);
            }
            else
            {
                DeepSearch(pParse,ppTemp,iNew,0,refset.nRef);
            }
        }


        if(i==iLoop&&iLoop<100)
        {
            iLoop++;
            i=-1;
        }
    }

    SetSameNode(pParse,&ppTest[3],ppTemp);
    SetSameNode(pParse,&ppTest[4],ppTemp);
    rc = SubstProp(pParse,ppTest[3]->pLeft,ppTest[4]);
    log_a("rc %d",rc);

    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    ClearSubstFlag(pParse,ppTest[3]);
    ClearSubstFlag(pParse,ppTest[4]);
    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    FreeAstTree(pParse,&ppTest[3],ppTemp);
    FreeAstTree(pParse,&ppTest[4],ppTemp);

    for(i=0; i<theoremset.n; i++)
    {
        FreeAstTree(pParse,&theoremset.data[i],ppTemp);
    }

}

void  SubstSingleTest(AstParse *pParse,TokenInfo **ppTest)
{
    int i;
    int n;
    int rc;
    TokenInfo *ppTemp[100];
    for(i=0; i<3; i++)
    {
        PrintAst(pParse,ppTest[i]);
        n = SetSameNode(pParse,&ppTest[i],ppTemp);
        log_a("n %d",n);
    }

    SetSameNode(pParse,&ppTest[3],ppTemp);
    SetSameNode(pParse,&ppTest[4],ppTemp);
    rc = SubstProp(pParse,ppTest[3]->pLeft,ppTest[4]);
    log_a("rc %d",rc);

    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    ClearSubstFlag(pParse,ppTest[3]);
    ClearSubstFlag(pParse,ppTest[4]);
    PrintSubstAst(pParse,ppTest[3]);
    PrintSubstAst(pParse,ppTest[4]);
    for(i=0; i<pParse->all_num; i++)
    {
        FreeAstTree(pParse,&ppTest[i],ppTemp);
    }
}


void  SubstMpTest(AstParse *pParse,TokenInfo **ppTest)
{
    int i;
    int n;
    TokenInfo *ppTemp[100];//存在递归时的共享变量
    TokenInfo *pR;//

    pParse->ppTemp = ppTemp;
    for(i=0; i<3; i++)
    {
        SetSameNode(pParse,&ppTest[i],ppTemp);
        printf("num:%d\n",i+1);
        PrintAst(pParse,theoremset.data[i]);
    }
  //  for(i=3;i<pParse->axiom_num;i++)
    for(i=3;i<pParse->all_num;i++)
    {
        log_a("old i %d",i+1);
        PrintAst(pParse,ppTest[i]);
        pR = PropMpSeq(pParse,ppTest,ppTest[i]);
        if(pR!=NULL){
            pR = CopyAstTree(pParse,pR,0);
            FreePropSeq(pParse,ppTest[i],ppTemp);
            FreeAstTree(pParse,&ppTest[i],ppTemp);
            ppTest[i] = pR;
        }
        log_a("new i %d",i+1);
        PrintAst(pParse,ppTest[i]);
        SetSameNode(pParse,&ppTest[i],ppTemp);
    }
    for(; i<pParse->all_num; i++)
    {
        printf("num:%d\n",i+1);
        PrintAst(pParse,ppTest[i]);
    }

   // for(i=0; i<pParse->all_num; i++)
    for(i=0; i<theoremset.n; i++)
    {
        FreeAstTree(pParse,&ppTest[i],ppTemp);
    }
    pParse->ppTemp = NULL;
}
