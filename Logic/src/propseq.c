#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include <assert.h>
#include "prop.h"

//#define MP_DEBUG

TokenInfo * PropMpSeq(AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo **ppTemp,
        TokenInfo *pSeq)
{
    TokenInfo *pLeft;
    TokenInfo *pRight;
    int iNum;
    static int cnt = 0;
    cnt++;
    assert(pSeq!=NULL);
    if( pSeq->type==PROP_SYMB )
    {
        //iNum = pSeq->symb - 'A';
        iNum = atoi(pSeq->zSymb)-1;
        //assert(iNum<3);
        pSeq->pTheorem = ppTest[iNum];
//        log_c("L%d:",iNum+1);
//        PrintSubstAst(pParse,pSeq->pTheorem);
        cnt--;
        return pSeq->pTheorem;
    }
    else if(pSeq->type==PROP_IMPL)
    {
        pLeft =  PropMpSeq(pParse,ppTest,ppTemp,pSeq->pLeft);
        pRight =  PropMpSeq(pParse,ppTest,ppTemp,pSeq->pRight);
#ifdef MP_DEBUG
        log_a("left");
        PrintAst(pParse,pLeft);
        log_a("right");
        PrintAst(pParse,pRight);
#endif
        if(pLeft!=NULL&&pRight!=NULL)
        {
            pSeq->pTheorem = PropMpSubst(pParse,ppTemp,pLeft,pRight);
#ifdef MP_DEBUG
            log_a("mp");
            PrintAst(pParse,pSeq->pTheorem);
#endif
        }
        else
        {
            log_a("mp null");
            pSeq->pTheorem = NULL;
        }
        cnt--;
        return pSeq->pTheorem;
    }
    else{
        assert(0);
    }
    cnt--;
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



