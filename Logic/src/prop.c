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

typedef struct PermData
{
    u8 nAll;
    u8 iNofree;
    u8 nFree;
    u8 pos;
    u8 aMap[10];
}PermData;


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

    if( pA->bSubst && pA->pSubst->type!=PROP_SYMB )
    {
        pA = pA->pSubst;
    }
    if( pB->bSubst && pB->pSubst->type!=PROP_SYMB )
    {
        pB = pB->pSubst;
    }

    if( pA->type==PROP_SYMB )
    {
        if( pB->type==PROP_SYMB )
        {
            if( !pA->bSubst )
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


void  SubstPropTest(
        AstParse *pParse,
        TokenInfo **ppTest)
{
    int n;
    int rc;
    TokenInfo *ppTemp[10];

    PrintAst(pParse,ppTest[0]);
    PrintAst(pParse,ppTest[1]);
    n = SetSameNode(pParse,&ppTest[0],ppTemp);
    log_a("n %d",n);
    n = SetSameNode(pParse,&ppTest[1],ppTemp);
    log_a("n %d",n);
    rc = SubstProp(pParse,ppTest[0]->pLeft,ppTest[1]);
    log_a("rc %d",rc);

    PrintSubstAst(pParse,ppTest[0]);
    PrintSubstAst(pParse,ppTest[1]);
}
