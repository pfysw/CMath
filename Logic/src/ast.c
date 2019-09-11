/*
 * ast.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include <assert.h>

void PrintAst(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    cnt++;
    assert(pAst!=NULL);
    if( pAst->type==PROP_SYMB )
    {
        log_c("%c",pAst->symb);
    }
    else if( pAst->type==PROP_NEG )
    {
        log_c("~");
        PrintAst(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        log_c("(");
        PrintAst(pParse,pAst->pLeft);
        log_c("->");
        PrintAst(pParse,pAst->pRight);
        log_c(")");
    }
    cnt--;
    if(!cnt)
    {
        log_a("");
    }
}

void PrintSubstAst(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    cnt++;
    assert(pAst!=NULL);

    if( pAst->type==PROP_SYMB )
    {
        if( pAst->bSubst )
        {
            if( pAst->pSubst->type==PROP_SYMB &&
                    !pAst->pSubst->bSubst )
            {
                log_c("%c",pAst->pSubst->symb);
            }
            else
            {
                PrintSubstAst(pParse,pAst->pSubst);
            }
        }
        else
        {
            log_c("%c",pAst->symb);
        }
    }
    else if( pAst->type==PROP_NEG )
    {
        log_c("~");
        PrintSubstAst(pParse,pAst->pLeft);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        log_c("(");
        PrintSubstAst(pParse,pAst->pLeft);
        log_c("->");
        PrintSubstAst(pParse,pAst->pRight);
        log_c(")");
    }
    cnt--;
    if(!cnt)
    {
        log_a("");
    }
}



void PrintAstAddr(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    cnt++;
    assert(pAst!=NULL);
    if( pAst->type==PROP_SYMB )
    {
        log_a("%c %p",pAst->symb,pAst);
        //log_c("sym %s type %d",temp,pAst->type);
    }
    else if( pAst->type==PROP_NEG )
    {
        log_a("~");
        PrintAstAddr(pParse,pAst->pLeft);
        //log_a("left %s|%p",pAst->pLeft->zSymb,pAst->pLeft->zSymb);
    }
    else
    {
        assert(pAst->type==PROP_IMPL);
        log_a("(");
        PrintAstAddr(pParse,pAst->pLeft);
        log_a("->");
        PrintAstAddr(pParse,pAst->pRight);
        log_a(")");
    }
    cnt--;
    if(!cnt)
    {
        log_a("");
    }
}

TokenInfo *NewNode(AstParse *pParse)
{
    TokenInfo *p;
    p = (TokenInfo *)malloc(sizeof(TokenInfo));
    pParse->malloc_cnt++;
    memset(p,0,sizeof(TokenInfo));
    //log_a("cnt %d",pParse->malloc_cnt);
    return p;
}
void FreeAstNode(AstParse *pParse,TokenInfo *p)
{
//    if(p->type==PROP_SYMB)
//    {
//        free(p->zSymb);
//    }
   // log_a("free %s",p->zSymb);
    pParse->free_cnt++;
    free(p);
}

void FreeAstTree(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp)
{
    static int cnt = 0;
    cnt++;
    static int n = 0;
    int i;

    assert((*ppAst)!=NULL);

    for(i=0;i<n;i++)
    {
        if( ppTemp[i]==*ppAst )
        {
            goto end;
        }
    }
    if( (*ppAst)->type==PROP_SYMB )
    {
        FreeAstNode(pParse,(*ppAst));
    }
    else if( (*ppAst)->type==PROP_NEG )
    {
        FreeAstTree(pParse,&((*ppAst)->pLeft),ppTemp);
        FreeAstNode(pParse,(*ppAst));
    }
    else
    {
        assert((*ppAst)->type==PROP_IMPL);
        FreeAstTree(pParse,&((*ppAst)->pLeft),ppTemp);
        FreeAstTree(pParse,&((*ppAst)->pRight),ppTemp);
        FreeAstNode(pParse,(*ppAst));
    }
    ppTemp[n++] = *ppAst;
end:
    *ppAst = NULL;
    cnt--;
    if(!cnt) n = 0;
}

void SetSymb(AstParse *pParse, TokenInfo *pB)
{
//分配字符串
#if 0
    char temp[10] = {0};
    assert(pB->nSymbLen<10);
    memcpy(temp,pB->zSymb,pB->nSymbLen);
    pB->zSymb = malloc(pB->nSymbLen);
    memcpy(pB->zSymb,temp,pB->nSymbLen);
    log_a("sym %s len %d",pB->zSymb,pB->nSymbLen);
#endif
    pB->type = PROP_SYMB;
}

void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB)
{
    pA->type = PROP_NEG;
    pA->pLeft = pB;
    //log_a("pB %s|%p",pB->zSymb,pB->zSymb);
}

void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC)
{
    pA->type = PROP_IMPL;
    pA->pLeft = pB;
    pA->pRight = pC;
}

TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst)
{
    TokenInfo *pDst;
    TokenInfo *pTemp;
    u32 testn;

    pDst = NewNode(pParse);

    pDst->type = pSrc->type;
    if( pSrc->type==PROP_SYMB )
    {
        if( bSubst && pSrc->bSubst )
        {
            pTemp = pSrc;
            if( pSrc->pSubst->type==PROP_SYMB  &&
                    !pSrc->pSubst->bSubst )
            {
                pDst->symb = pSrc->pSubst->copy;
               // pDst->symb = pSrc->pSubst->symb;
            }
            else
            {
                //如果不是PROP_SYMB，那么pSrc->pSubst->bSubst不可能是1
                while( pSrc->pSubst->bSubst )
                {
                    assert( pSrc->pSubst->type==PROP_SYMB );
                    pSrc = pSrc->pSubst;
                }
                if( pSrc->pSubst->type==PROP_SYMB )
                {
                    pDst->symb = pSrc->pSubst->copy;
                   //pDst->symb = pSrc->pSubst->symb;
                }
                else
                {
                    pDst->type = pSrc->pSubst->type;
                    pDst->pLeft = CopyAstTree(pParse,
                            pSrc->pSubst->pLeft,bSubst);
                    if( pSrc->pSubst->type==PROP_IMPL )
                    {

                        pDst->pRight = CopyAstTree(pParse,
                                pSrc->pSubst->pRight,bSubst);
                    }
                }
            }
        }
        else
        {
            if( bSubst )
                pDst->symb = pSrc->copy;
            else
                pDst->symb = pSrc->symb;
        }
    }
    else
    {
        pDst->pLeft = CopyAstTree(pParse,pSrc->pLeft,bSubst);
        if( pSrc->type==PROP_IMPL )
            pDst->pRight = CopyAstTree(pParse,pSrc->pRight,bSubst);
    }

    return pDst;
}
