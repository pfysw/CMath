#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>
#include "prop.h"
#include "propseq.h"

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
            assert(pSeq->zSymb!=NULL);
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
            pSeq->pDeduce = PropAdd(pParse,ppTest,pSeq);
            log_a("add pDeduce %d",cnt);
            PrintAst(pParse,pSeq->pDeduce);
            pSeq->pTheorem = PropMpSeq(pParse,ppTest,pSeq->pDeduce);
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
    TokenInfo *pRight;
    TokenInfo *apCopy[5] = {0};
    TokenInfo **ppAxiom = pParse->apAxiom;
    static int cnt = 0;
    cnt++;

    assert(pSeq->op==OP_ADD);
    if(pSeq->pRight->type==PROP_SYMB){
        if(pSeq->pRight==pSeq->pLeft){
            apCopy[0] = NewImplyNode(pParse,ppAxiom[0],ppAxiom[1],">");
            pR = NewImplyNode(pParse,ppAxiom[0],apCopy[0],">");
        }
        else{
            pR = NewImplyNode(pParse,pSeq->pRight,ppAxiom[0],">");
        }
#ifdef ADD_DEBUG
        log_a("add pR");
        PrintAst(pParse,pR);
#endif
        cnt--;
        return pR;
    }
    assert(pSeq->pRight->type==PROP_IMPL);
    if(pSeq->pRight->isDeduction && pSeq->pRight->op!=OP_ADD)
    {
        pRight = PropRemoveAdd(pParse,ppTest,pSeq->pRight);
        pRl = pRight->pLeft;
        pRr = pRight->pRight;
    }
    else
    {
        pRl = pSeq->pRight->pLeft;
        pRr = pSeq->pRight->pRight;
        pRight = pSeq->pRight;
    }
    switch(pSeq->pRight->op)
    {
    case OP_MP:
        //if(pSeq->pLeft==pRl && !pRr->isDeduction )
        if(pSeq->pLeft==pRl && (!pRr->isDeduction || !isChildProp(pParse,pRr,pSeq->pLeft)) )
        {
            pR = pRr;
#ifdef ADD_DEBUG
            log_a("add pR1 %d",cnt);
            PrintAst(pParse,pR);
#endif
        }
        else if(!isChildProp(pParse,pRight,pSeq->pLeft)){
            pR = NewImplyNode(pParse,pRight,ppAxiom[0],">");
        }
        else
        {
            apCopy[0] = NewImplyNode(pParse,pSeq->pLeft,pRl,"+");
#ifdef ADD_DEBUG
            log_a("add left %d",cnt);
            PrintAst(pParse,apCopy[0]);
#endif
            pNl = PropAdd(pParse,ppTest,apCopy[0]);
            apCopy[1] = NewImplyNode(pParse,pSeq->pLeft,pRr,"+");
#ifdef ADD_DEBUG
            log_a("add right %d",cnt);
            PrintAst(pParse,apCopy[1]);
#endif
            apCopy[2] = PropAdd(pParse,ppTest,apCopy[1]);
            pNr = NewImplyNode(pParse,apCopy[2],ppAxiom[1],">");
            pR = NewImplyNode(pParse,pNl,pNr,">");//nl,nr,apCopy[2]都作为pR的子结点
            FreeAstNode(pParse,apCopy[0]);
            FreeAstNode(pParse,apCopy[1]);
#ifdef ADD_DEBUG
            log_a("add pR2 %d",cnt);
            PrintAst(pParse,pR);
#endif
        }
        break;
    case OP_HS:
        //if(pSeq->pLeft==pRl && !pRr->isDeduction )
        if(pSeq->pLeft==pRl && (!pRr->isDeduction || !isChildProp(pParse,pRr,pSeq->pLeft)) )
        {
            apCopy[0] = NewImplyNode(pParse,pRr,ppAxiom[0],">");
            pR = NewImplyNode(pParse,apCopy[0],ppAxiom[1],">");;
        }
        else if(!isChildProp(pParse,pRight,pSeq->pLeft)){
            pR = NewImplyNode(pParse,pRight,ppAxiom[0],">");
        }
        else if(pSeq->pLeft==pRr){
            apCopy[0] = NewImplyNode(pParse,pSeq->pLeft,pRl,"+");
            pNl = PropAdd(pParse,ppTest,apCopy[0]);
            apCopy[1] = NewImplyNode(pParse,ppAxiom[0],ppAxiom[1],">>");
            pNr = NewImplyNode(pParse,apCopy[1],ppAxiom[1],">");
            pR = NewImplyNode(pParse,pNl,pNr,">");
        }
        else{
            apCopy[4] = NewImplyNode(pParse,pSeq->pLeft,pRl,"+");
            pNl = PropAdd(pParse,ppTest,apCopy[4]);
            apCopy[0] = NewImplyNode(pParse,pRr,ppAxiom[0],">");//todo 要不要释放
            apCopy[1] = NewImplyNode(pParse,apCopy[0],ppAxiom[1],">");
            apCopy[2] = NewImplyNode(pParse,pSeq->pLeft,apCopy[1],"+");
            apCopy[3] = PropAdd(pParse,ppTest,apCopy[2]);
            pNr = NewImplyNode(pParse,apCopy[3],ppAxiom[1],">");
            pR = NewImplyNode(pParse,pNl,pNr,">");//nl,nr,apCopy[3]都作为pR的子结点
            FreeAstNode(pParse,apCopy[4]);
            FreeAstNode(pParse,apCopy[2]);
        }
        break;
    case OP_ADD:
        apCopy[0] = PropAdd(pParse,ppTest,pSeq->pRight);
        apCopy[1] = NewImplyNode(pParse,pSeq->pLeft,apCopy[0],"+");
        pR =  PropAdd(pParse,ppTest,apCopy[1]);
        break;
    default:
        assert(0);
        break;
    }

#ifdef ADD_DEBUG
        log_a("add pR");
        PrintAst(pParse,pR);
#endif
    cnt--;
    return pR;
}

TokenInfo * PropRemoveAdd(
        AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq)
{
    assert(pSeq->type==PROP_IMPL);


    TokenInfo *pR;
    TokenInfo *apCopy[5] = {0};

    assert(pSeq->op!=OP_ADD);
    apCopy[0] = PropRemoveAdd(pParse,ppTest,pSeq->pLeft);
    apCopy[1] = PropRemoveAdd(pParse,ppTest,pSeq->pRight);
    pR = NewImplyNode(pParse,apCopy[0],apCopy[1],pSeq->zSymb);
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



