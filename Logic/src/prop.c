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
    assert( cnt<10 );
    if(  (*ppAst)->type==PROP_SYMB )
    {

        if( (*ppAst)->bSubst && isSubst &&
                ((*ppAst)->pSubst->type!=PROP_SYMB  ||
                        (*ppAst)->pSubst->bSubst ) )
        {
            GetDiffNode(pParse,&((*ppAst)->pSubst),ppTemp,isSubst);
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
                ppTemp[j]->symb = 'A'+i;//在这个位置上变量所有非自由变元
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
    PermPropSymb(pParse,pParse->pRoot,ppTemp,n,0);
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

#define DEBUG 0

Vector theoremset;


int GetAddrTest(void)
{
    int rc = 0;
    int test;
    test = theoremset.data[51];
    if( theoremset.n>90 )
    {
        if( (test>10000)||(test<-10000))
        {
            rc = 1;
        }
    }
    else
        rc = 1;
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
    TokenInfo *apCopy[5];

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
    for(i=0; i<20; i++)
    {
        if( theoremset.data[i]->isRightTheorem ) continue;

        for(j=0; j<20&&j<theoremset.n; j++)
        {
            if( i==5 && j>0 ) break;
            //if( theoremset.n>51 && theoremset.data[51]<100 )
            if( i==13 && j==19 )
            {
                log_a("tt");
            }
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
                n = GetDiffNode(pParse,&theoremset.data[i]->pRight,ppTemp,1);
#if DEBUG&&0
                log_a("n %d",n);
#endif
                if( n>3 ) goto end_insert;
                m = GetAllNode(pParse,&theoremset.data[i]->pRight,ppTemp);
#if DEBUG&&0
                log_a("m %d",m);
#endif
                if( m>10 ) goto end_insert;

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
                assert( GetAddrTest() );
                InsertVector(&theoremset,apCopy[1]);
                assert( GetAddrTest() );
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
                log_a("j: %d",j);
                PrintAst(pParse,theoremset.data[j]);
                log_a("----");
#endif
                log_a("num %d",theoremset.n);
                PrintSubstAst(pParse,apCopy[1]);
            }
        end_insert:
            ClearSubstFlag(pParse,theoremset.data[i]);
            ClearSubstFlag(pParse,theoremset.data[j]);
            if( i==j )
            {
                FreeAstTree(pParse,&apCopy[0],ppTemp);
            }
        }
    }


    SetSameNode(pParse,&ppTest[3],ppTemp);
    SetSameNode(pParse,&ppTest[4],ppTemp);
    rc = SubstProp(pParse,ppTest[3],ppTest[4]);
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


