#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include <assert.h>
#include "prop.h"

//#define MP_DEBUG

TokenInfo * PropAdd(AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq);

TokenInfo * PropMpSeq(AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq)
{
    TokenInfo *pLeft;
    TokenInfo *pRight;
    TokenInfo *pTemp;
    TokenInfo **ppTemp = pParse->ppTemp;
    int iNum;
    static int cnt = 0;
    cnt++;
    assert(pSeq!=NULL);
    if( pSeq->type==PROP_SYMB )
    {
        if(pSeq->symb<='9' && pSeq->symb>'0'){
            iNum = atoi(pSeq->zSymb)-1;
            pSeq->pTheorem = ppTest[iNum];
    //        log_c("L%d:",iNum+1);
    //        PrintSubstAst(pParse,pSeq->pTheorem);
        }
        else{
            pSeq->pTheorem = NULL;
        }
        cnt--;
        return pSeq->pTheorem;
    }
    else if(pSeq->type==PROP_IMPL)
    {
        if(pSeq->op!=OP_ADD)
        {
            pLeft =  PropMpSeq(pParse,ppTest,pSeq->pLeft);
            pRight =  PropMpSeq(pParse,ppTest,pSeq->pRight);
    #ifdef MP_DEBUG
            log_a("left");
            PrintAst(pParse,pLeft);
            log_a("right");
            PrintAst(pParse,pRight);
    #endif
            if(pLeft!=NULL&&pRight!=NULL)
            {
                if(pSeq->op==OP_HS){
                    pSeq->pTheorem = PropMpSubst(pParse,ppTemp,pRight,ppTest[0]);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,ppTemp,pTemp,ppTest[1]);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,ppTemp,pLeft,pTemp);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                }
                else{
                    pSeq->pTheorem = PropMpSubst(pParse,ppTemp,pLeft,pRight);
                }
    #ifdef MP_DEBUG
                log_a("mp %d",pSeq->op);
                PrintAst(pParse,pSeq->pTheorem);
    #endif
            }
            else
            {
                log_a("mp null");
                pSeq->pTheorem = NULL;
            }
        }
        else{
            PropAdd(pParse,ppTest,pSeq);
        }
        cnt--;
        return pSeq->pTheorem;
    }
    else{
       // assert(0);
        return NULL;
    }
    cnt--;
    return NULL;
}

TokenInfo * PropAdd(
        AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq)
{
    return NULL;
}

void FreePropSeq(AstParse *pParse,TokenInfo *pSeq,TokenInfo **ppTemp)
{
    assert(pSeq!=NULL);
    if( pSeq->type==PROP_SYMB )
    {
        return;
    }
    else if(pSeq->type==PROP_IMPL)
    {
        if(pSeq->pTheorem!=NULL){
            FreeAstTree(pParse,&pSeq->pTheorem,ppTemp);
        }
        FreePropSeq(pParse,pSeq->pLeft,ppTemp);
        FreePropSeq(pParse,pSeq->pRight,ppTemp);
    }
    else{
        assert(0);
    }
}



