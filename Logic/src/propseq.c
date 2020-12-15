#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
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
                    pSeq->pTheorem = PropMpSubst(pParse,pRight,ppTest[0]);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,pTemp,ppTest[1]);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,pLeft,pTemp);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                }
                else{
                    pSeq->pTheorem = PropMpSubst(pParse,pLeft,pRight);
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
            //PropAdd(pParse,ppTest,pSeq);
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
    assert(pSeq->pLeft->type==PROP_SYMB);

    TokenInfo *pRl;
    TokenInfo *pRr;
    TokenInfo *pNl;
    TokenInfo *pNr;
    TokenInfo *pR;
    TokenInfo *apCopy[5] = {0};
    TokenInfo **ppTemp = pParse->ppTemp;
    TokenInfo **ppAxiom = pParse->apAxiom;
    char aNum[] = "123";

    assert(pSeq->op==OP_ADD);
    if(pSeq->pRight->type==PROP_SYMB){
        if(pSeq->pRight==pSeq->pLeft){
            apCopy[0] = NewMpNode(pParse,ppAxiom[0],ppAxiom[1]);
            apCopy[1] = NewMpNode(pParse,ppAxiom[0],apCopy[0]);
            pR = NewMpNode(pParse,apCopy[1],ppAxiom[0]);
        }
        else{
            pR = NewMpNode(pParse,pSeq->pRight,ppAxiom[0]);
        }
        return pR;
    }
    assert(pSeq->pRight->type==PROP_IMPL);
    pRl = pSeq->pRight->pLeft;
    pRr = pSeq->pRight->pRight;
    switch(pSeq->pRight->op)
    {
    case OP_MP:
        if(pSeq->pLeft==pRl && !pRr->isDeduction)
        {
            pR = pRr;
        }
        else
        {
            apCopy[0] = NewAddNode(pParse,pSeq->pLeft,pRl);
            pNl = PropAdd(pParse,ppTest,apCopy[0]);
            apCopy[1] = NewAddNode(pParse,pSeq->pLeft,pRr);
            apCopy[2] = PropAdd(pParse,ppTest,apCopy[1]);
            pNr = NewMpNode(pParse,apCopy[2],ppAxiom[1]);
            pR = NewMpNode(pParse,pNl,pNr);//nl,nr,apCopy[2]都作为pR的子结点

            FreeAstTree(pParse,&apCopy[0],ppTemp);
            FreeAstTree(pParse,&apCopy[1],ppTemp);
        }
        break;
    case OP_HS:
        if(pSeq->pLeft==pRl){
            if(pSeq->pRight!=pRl){
                apCopy[0] = PropMpSubst(pParse,pRr,ppTest[0]);
                apCopy[1] = PropMpSubst(pParse,apCopy[0],ppTest[1]);
                pR = apCopy[1];
                FreeAstTree(pParse,&apCopy[0],ppTemp);
            }
            else{
                printf("A hs A can't get result\n");
                assert(0);
            }
        }
        else if(pSeq->pLeft==pRr){
            apCopy[2] = PropMpSeq(pParse,ppTest,pRl);//公理集的拷贝，不要释放
            pNl = PropMpSubst(pParse,apCopy[2],ppTest[0]);
            apCopy[0] = NewNode(pParse);
            SetImplExpr(pParse,apCopy[0],ppTest[0],ppTest[1],NULL);
            apCopy[0]->op = OP_HS;
            apCopy[1] = PropMpSeq(pParse,ppTest,apCopy[0]);
            pNr = PropMpSubst(pParse,apCopy[1],ppTest[1]);
            FreeAstTree(pParse,&pNl,ppTemp);
            FreeAstTree(pParse,&pNr,ppTemp);
            FreeAstTree(pParse,&apCopy[0],ppTemp);
            FreeAstTree(pParse,&apCopy[1],ppTemp);
        }
        else{
            apCopy[0] = PropMpSubst(pParse,pRl,pRr);
            pR = PropMpSubst(pParse,apCopy[0],ppTest[0]);
            FreeAstTree(pParse,&apCopy[0],ppTemp);
        }
        break;
    case OP_ADD:
        apCopy[0] = PropAdd(pParse,ppTest,pSeq->pRight);
        pNl = NewMpNode(pParse,pSeq->pLeft,apCopy[0]->pLeft);
        apCopy[1] = NewMpNode(pParse,pSeq->pLeft,apCopy[0]->pRight);
        pNr = NewMpNode(pParse,apCopy[1],ppAxiom[1]);
        pR = NewMpNode(pParse,pNl,pNr);
        break;
    default:
        assert(0);
        break;
    }
    return pR;
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



