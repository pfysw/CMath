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

typedef struct PermData
{
    u8 nAll;
    u8 iNofree;
    u8 nFree;
    u8 pos;
    u8 aMap[10];
}PermData;

typedef struct vector
{
    TokenInfo **data;
    int n;
    int size;
}Vector;

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
    TokenInfo *pTmp;
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
                FreeAstNode(pParse,(*ppAst));
                *ppAst = ppTemp[i];
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
    assert( cnt<20 );
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
    TokenInfo **temp;
    if( pV->n<pV->size )
    {
        pV->data[pV->n++] = pData;
    }
    else
    {
        assert( pV->size<1000000 );

//        temp = malloc(pV->size*2);
//        memset(temp,0,sizeof(TokenInfo **)*pV->size*2);
//        memcpy(temp,pV->data,sizeof(TokenInfo **)*pV->size);
//        free(pV->data);
//        pV->data = temp;
//        pV->size = pV->size*2;
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
#define INDEX_I   115
#define INDEX_J   55
#define NUM_NOT_SAME   4
#define LOOP_N         5

Vector theoremset;
u8 aCnt[1000] = {0};

#if 0
void  SubstPropTest1(
        AstParse *pParse,
        TokenInfo **ppTest)
{
    int i,j,k;
    int n,m;
    int rc;
    TokenInfo *ppTemp[100];
    TokenInfo *apCopy[5];
    int iLoop = 0;

    memset(&theoremset,0,sizeof(Vector));
    theoremset.size = 100;
    theoremset.data = malloc(theoremset.size*sizeof(TokenInfo **));

    for(i=0; i<3; i++)
    {
        PrintAst(pParse,ppTest[i]);
        n = SetSameNode(pParse,&ppTest[i],ppTemp);
        log_a("n %d",n);
        InsertVector(&theoremset,ppTest[i]);
        PrintAst(pParse,theoremset.data[i]);
    }
    log_a("***********");
    for(i=0; i<INDEX_I||(i=0,(iLoop++<LOOP_N)); i++)
    {
        if( i>=theoremset.n ) continue;
        if( 1==theoremset.data[i]->isRightTheorem )
        {
            continue;
        }

        for(j=aCnt[i]; (j<aCnt[i]+INDEX_J)&&j<theoremset.n; j++)
        {
            //assert(j<INDEX_J);
            //if(j==2||j==1) continue;
            //if( i==5 && j>0 ) break;
            if( theoremset.data[i]->isRightTheorem && j )
            {
                break;
            }
            //if( theoremset.n>51 && theoremset.data[51]<100 )
//            if( i==57 )//&& j==1 )
//            {
//                PrintAst(pParse,theoremset.data[i]);
//                log_a("tt");
//                assert(0);
//            }
            n = 0;
            if(i==j)
            {
                apCopy[0] = CopyAstTree(pParse,theoremset.data[i],0);
                SetSameNode(pParse,&apCopy[0],ppTemp);
                rc = SubstProp(pParse,theoremset.data[i]->pLeft,apCopy[0]);
            }
            else
            {
                rc = SubstProp(pParse,
                        theoremset.data[i]->pLeft,theoremset.data[j]);
            }
#if DEBUG
           // log_a("rc %d i %d j %d",rc ,i,j);
#endif
            if( rc )
            {
//                if(theoremset.n==839)
//                {
//                    log_a("sa %d",theoremset.n);
//                }
//                PrintSubstAst(pParse,theoremset.data[i]->pRight);

                m = GetAllNode(pParse,&theoremset.data[i]->pRight,ppTemp);
#if DEBUG&&0
                log_a("m %d",m);
#endif
                if( m>10 ) goto end_insert;

                n = GetDiffNode(pParse,&theoremset.data[i]->pRight,ppTemp,1);
#if DEBUG&&0
                log_a("n %d",n);
#endif
                if( n>NUM_NOT_SAME ) goto end_insert;


                for(k=0; k<n; k++)
                {
                    //log_a("sym %c",ppTemp[k]->symb);
                    //ppTemp[k]->symb = 'A'+k;
                    ppTemp[k]->copy = 'A'+k;
                }

                apCopy[1] = CopyAstTree(pParse,theoremset.data[i]->pRight,1);

                SetSameNode(pParse,&apCopy[1],ppTemp);
                if( 0==i )
                {
                    apCopy[1]->isRightTheorem = 1;
                }
                else if( i==1 && 0==j )
                {
                    apCopy[1]->isRightTheorem = 2;
                }

                InsertVector(&theoremset,apCopy[1]);

#if DEBUG
                n = GetDiffNode(pParse,&theoremset.data[i],ppTemp,0);
                for(k=0; k<n; k++)
                {
                    //log_a("sym %c",ppTemp[k]->symb);
                    ppTemp[k]->symb = 'A'+k;
                }
                //PrintSubstAst(pParse,theoremset.data[i]);
                log_a("i: %d",i);
                PrintAst(pParse,theoremset.data[i]);
                //PrintSubstAst(pParse,theoremset.data[i]);
                log_a("j: %d",j);
                PrintAst(pParse,theoremset.data[j]);
               // PrintSubstAst(pParse,theoremset.data[j]);
                log_a("----");
#endif
                if( n<NUM_NOT_SAME+1 )
                {
                    log_a("num %d",theoremset.n);
                    PrintSubstAst(pParse,apCopy[1]);
                }
            }
        end_insert:
            ClearSubstFlag(pParse,theoremset.data[i]);
            ClearSubstFlag(pParse,theoremset.data[j]);
            if( i==j )
            {
                FreeAstTree(pParse,&apCopy[0],ppTemp);
            }
        }
        aCnt[i] = j;
        if(i==14)
        {
            log_a("s");
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
#endif


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
    if( n>NUM_NOT_SAME ) return 0;


    for(k=0; k<n; k++)
    {
        //log_a("sym %c",ppTemp[k]->symb);
        //ppTemp[k]->symb = 'A'+k;
        ppTemp[k]->copy = 'A'+k;
    }

    apCopy[1] = CopyAstTree(pParse,theoremset.data[j]->pRight,1);

    SetSameNode(pParse,&apCopy[1],ppTemp);

    if( 0==j )
    {
        apCopy[1]->isRightTheorem = 1;
    }
    else if( 0==i )
    {
        if(j==1)
            apCopy[1]->isRightTheorem = 2;
        else if(4==j)
        {
            apCopy[1]->isRightTheorem = 1;
        }
    }

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
    if( n>NUM_NOT_SAME )
    {
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
    for(k=0; k<n; k++)
    {
        ppTemp[k]->symb = 'A'+k;
    }
    n = GetDiffNode(pParse,&theoremset.data[i],ppTemp,0);
    for(k=0; k<n; k++)
    {
        ppTemp[k]->symb = 'A'+k;
    }
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

    if( 1==theoremset.data[j]->isRightTheorem )
    {
        return 0;
    }

    if( theoremset.data[j]->isRightTheorem && i )
    {
        return 0;
    }

    n = 0;
    if(i==j)
    {


        apCopy[0] = CopyAstTree(pParse,theoremset.data[i],0);
        SetSameNode(pParse,&apCopy[0],ppTemp);

        if( hs )
        {
            if( apCopy[0]->type!=PROP_IMPL )
            {
                goto end_insert;
            }
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

#if DEBUG

        //PrintSubstAst(pParse,theoremset.data[i]);
        log_a("i: %d",i);
        PrintAst(pParse,theoremset.data[i]);
        //PrintSubstAst(pParse,theoremset.data[i]);
        log_a("j: %d",j);
        PrintAst(pParse,theoremset.data[j]);
       // PrintSubstAst(pParse,theoremset.data[j]);
        log_a("----");
#endif
        if( n<NUM_NOT_SAME+1 )
        {
            log_a("num %d",theoremset.n);
            PrintSubstAst(pParse,apCopy[1]);
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

void  SubstPropTest(
        AstParse *pParse,
        TokenInfo **ppTest)
{
    int i,j,k;
    int n,m;
    int rc;
    TokenInfo *ppTemp[100];
    int iLoop = 0;
    int nNow;

    memset(&theoremset,0,sizeof(Vector));
    theoremset.size = 100;
    theoremset.data = malloc(theoremset.size*sizeof(TokenInfo **));

    for(i=0; i<3; i++)
    {
        PrintAst(pParse,ppTest[i]);
        n = SetSameNode(pParse,&ppTest[i],ppTemp);
        log_a("n %d",n);
        InsertVector(&theoremset,ppTest[i]);
        PrintAst(pParse,theoremset.data[i]);
    }
    log_a("***********");
    for(i=0; i<INDEX_I&&i<theoremset.n; i++)
    {
        //for(j=aCnt[i]; (j<INDEX_J)&&j<theoremset.n; j++)
        for(j=aCnt[i]; (j<3)&&j<theoremset.n; j++)
        {
            rc = MpRule(pParse,ppTemp,i,j,0);
            if( rc )
            {
                nNow = theoremset.n-1;
                for(k=0;k<3;k++)
                {
                    int r;
                    //公理做前件
                    r = MpRule(pParse,ppTemp,k,nNow,0);
                    if( r )
                    {
                        nNow = theoremset.n-1;

                        for(int k=0;k<3;k++)
                        {
                            //HS
                            MpRule(pParse,ppTemp,nNow,k,1);
                            MpRule(pParse,ppTemp,k,nNow,1);
                        }
                    }

                }

            }
        }
        aCnt[i] = j;
        if(i==iLoop&&iLoop<100)
        {
            iLoop++;
            i=-1;
        }
    }

testdebug:
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


