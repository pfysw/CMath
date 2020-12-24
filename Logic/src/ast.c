/*
 * ast.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>

u8 testbuf[10000] = {0};
void PrintAst(AstParse *pParse,TokenInfo *pAst)
{
    static int cnt = 0;
    static int nPrintSymb = 0;
    cnt++;
    if(cnt==1){
        nPrintSymb = 0;
    }
    assert(pAst!=NULL);
    if( pAst->type==PROP_SYMB )
    {
        if(pAst->zSymb!=NULL){
            log_c("%s",pAst->zSymb);
        }
        else{
            log_c("%c",pAst->symb);
        }
        nPrintSymb++;
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
        //log_c("->");
        if(pAst->zSymb!=NULL){
            log_c("%s",pAst->zSymb);
        }
        else{
            log_c("->");
        }
        if(nPrintSymb%10==0){
            log_a("");
        }
        PrintAst(pParse,pAst->pRight);
        log_c(")");

    }
    cnt--;
    if(!cnt)
    {
        log_a("");
        nPrintSymb = 0;
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

TokenInfo *NewNode(AstParse *pParse)
{
    TokenInfo *p;
    p = (TokenInfo *)malloc(sizeof(TokenInfo));
    pParse->malloc_cnt++;
    memset(p,0,sizeof(TokenInfo));
#ifdef FREE_TEST
    p->malloc_flag = pParse->malloc_cnt;
    testbuf[pParse->malloc_cnt] = 1;
    if(pParse->malloc_cnt==2282){
        printf("newnode %d\n",pParse->malloc_cnt);
    }
#endif
    return p;
}
void FreeAstNode(AstParse *pParse,TokenInfo *p)
{
    if(pParse->free_cnt==2261)
    {
        printf("sd %d\n",pParse->free_cnt);
    }
    assert(p!=NULL);
    if(p->type==PROP_SYMB || p->type==PROP_IMPL)
    {
        if(p->zSymb!=NULL){
#ifdef FREE_TEST
            testbuf[p->malloc_string] = 0;
#endif
            free(p->zSymb);
            p->zSymb = NULL;
            pParse->free_cnt++;
        }
    }
#ifdef FREE_TEST
    testbuf[p->malloc_flag] = 0;
    if(p->malloc_flag==2282){
        log_a("free");
    }
#endif
    pParse->free_cnt++;
    free(p);
}

TokenInfo *NewTempNode(AstParse *pParse)
{
    TokenInfo *p;
    Mem5Global *pMem = pParse->pMem;
    p = (TokenInfo *)memsys5Malloc(pMem,sizeof(TokenInfo));
    memset(p,0,sizeof(TokenInfo));
    return p;
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
        assert(n<90);
        ppTemp[n++] = *ppAst;
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

end:
    *ppAst = NULL;
    cnt--;
    if(!cnt) n = 0;
}

void FreeNewImplyNodes(AstParse *pParse,TokenInfo **ppAst)
{
    assert((*ppAst)!=NULL);

    if((*ppAst)->type==PROP_IMPL)
    {
        FreeNewImplyNodes(pParse,&((*ppAst)->pLeft));
        FreeNewImplyNodes(pParse,&((*ppAst)->pRight));
        if((*ppAst)->isNewTemp){
            FreeAstNode(pParse,(*ppAst));
        }
        *ppAst = NULL;
    }

}

void NewSymbString(AstParse *pParse,TokenInfo *p)
{
    char temp[100] = {0};
    assert(p->nSymbLen<10);
    memcpy(temp,p->zSymb,p->nSymbLen);
    p->zSymb = malloc(p->nSymbLen+1);
    pParse->malloc_cnt++;
    memcpy(p->zSymb,temp,p->nSymbLen+1);
   // printf("symb %d\n",pParse->malloc_cnt);
#ifdef FREE_TEST
    p->malloc_string = pParse->malloc_cnt;
    testbuf[pParse->malloc_cnt] = 1;
    if(pParse->malloc_cnt==2264){
        printf("symb %d\n",pParse->malloc_cnt);
    }
#endif
}

void SetSymb(AstParse *pParse, TokenInfo *pB)
{
//分配字符串
    NewSymbString(pParse,pB);
    pB->type = PROP_SYMB;
}

AstParse *CreatAstParse(void){
    AstParse *pParse;
    char aNum[] = "123";
    int i = 0;

    pParse = (AstParse *)malloc(sizeof(AstParse));
    memset(pParse,0,sizeof(AstParse));
    for(i=0;i<3;i++){
        pParse->apAxiom[i] = NewNode(pParse);
        pParse->apAxiom[i]->symb = aNum[i];
        pParse->apAxiom[i]->type = PROP_SYMB;
        pParse->apAxiom[i]->zSymb = &aNum[i];
        pParse->apAxiom[i]->nSymbLen = 1;
        NewSymbString(pParse,pParse->apAxiom[i]);
    }

    return pParse;
}

void NewMemPool(AstParse *pParse,int len)
{
    pParse->pMem = memsys5Init(len,16);
    pParse->malloc_cnt++;
}

void FreeMemPool(AstParse *pParse)
{
    memsys5Shutdown(&pParse->pMem);
    pParse->free_cnt++;
}

void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB)
{
    pA->type = PROP_NEG;
    pA->pLeft = pB;
    //log_a("pB %s|%p",pB->zSymb,pB->zSymb);
}

TokenInfo * NewNegNode(AstParse *pParse,TokenInfo *pB)
{
    TokenInfo *pA =  NewTempNode(pParse);
    SetNegExpr(pParse,pA,pB);
    return pA;
}

void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        TokenInfo *pD)
{
    if(pD!=NULL){
        pA->zSymb = pD->zSymb;
        pA->nSymbLen = pD->nSymbLen;
        NewSymbString(pParse,pA);
        //printf("zSymb %s",pD->zSymb);
        if(!strcmp(pD->zSymb,"->")){
            pA->op = OP_IMPL;
        }
        else if(!strcmp(pD->zSymb,">")){
            pA->op = OP_MP;
        }
        else if(!strcmp(pD->zSymb,">>")){
            pA->op = OP_HS;
        }
        else if(!strcmp(pD->zSymb,"+")){
            pA->op = OP_ADD;
            pA->isDeduction = 1;
        }
        else{
            assert(0);
        }
        //pB->op = pA->op;
        //pC->op = pA->op;
    }

    pA->type = PROP_IMPL;
    pA->pLeft = pB;
    pA->pRight = pC;
    if(pB->isDeduction||pC->isDeduction)
    {
        pA->isDeduction = 1;
    }
}

TokenInfo * NewImplyNode(
        AstParse *pParse,
        TokenInfo *pB,
        TokenInfo *pC,
        char *zSymb)
{
    TokenInfo *pA =  NewTempNode(pParse);
    //printf("new \n");
    SetImplExpr(pParse,pA,pB,pC,NULL);
    pA->zSymb = zSymb;
    pA->nSymbLen = strlen(zSymb);
    if(!strcmp(zSymb,">")){
        pA->op = OP_MP;
    }
    else if(!strcmp(zSymb,">>")){
        pA->op = OP_HS;
    }
    else if(!strcmp(zSymb,"+")){
        pA->op = OP_ADD;
    }
    pA->isNewTemp = 1;
    //printf("end %d\n",pParse->test);
    return pA;
}

TokenInfo * NewSymbNode(AstParse *pParse,char *zSymb)
{
    TokenInfo *pA =  NewTempNode(pParse);
    pA->zSymb = zSymb;
    pA->nSymbLen = strlen(zSymb);
    pA->symb = zSymb[0];
    pA->type = PROP_SYMB;
    return pA;
}

TokenInfo * NewNumNode(AstParse *pParse,int num)
{
    TokenInfo *pA;
    char zSymb[10] = {0};
    sprintf(zSymb,"%d",num);
    pA = NewSymbNode(pParse,zSymb);
    return pA;
}

TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst)
{
    TokenInfo *pDst;

    pDst = NewNode(pParse);

    pDst->type = pSrc->type;
    if( pSrc->type==PROP_SYMB )
    {
        if( bSubst && pSrc->bSubst )
        {
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
