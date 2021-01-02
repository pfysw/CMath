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
                    log_a("mp1 %d",pSeq->op);
                    PrintAst(pParse,pSeq->pTheorem);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,pTemp,ppTest[1]);
                    log_a("mp2 %d",pSeq->op);
                    PrintAst(pParse,pSeq->pTheorem);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                    pTemp = pSeq->pTheorem;
                    pSeq->pTheorem = PropMpSubst(pParse,pLeft,pTemp);
                    FreeAstTree(pParse,&pTemp,ppTemp);
                }
                else{
                    pSeq->pTheorem = PropMpSubst(pParse,pLeft,pRight);
                }
    #if 1//def MP_DEBUG
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
            pTemp = CopyAstTree(pParse,pSeq->pTheorem,0);
            FreePropSeq(pParse,pSeq->pDeduce,ppTemp);
            //FreeNewImplyNodes(pParse,&pSeq->pDeduce);//不用了，改为内存池
            pSeq->pTheorem = pTemp;
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
            if(pRight->isNewTemp){
                //FreeAstNode(pParse,pRight);
            }
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
            if(pRight->isNewTemp){
                //FreeAstNode(pParse,pRight);
            }
            //FreeAstNode(pParse,apCopy[0]);
            //FreeAstNode(pParse,apCopy[1]);

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
            //FreeAstNode(pParse,apCopy[0]);
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
            //FreeAstNode(pParse,apCopy[4]);
            //FreeAstNode(pParse,apCopy[2]);
        }
        break;
    case OP_ADD:
        apCopy[0] = PropAdd(pParse,ppTest,pSeq->pRight);
        apCopy[1] = NewImplyNode(pParse,pSeq->pLeft,apCopy[0],"+");
        pR =  PropAdd(pParse,ppTest,apCopy[1]);
        ////FreeAstNode(pParse,apCopy[0]); //in mp pSeq->pRight free
        //FreeAstNode(pParse,apCopy[1]);
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
//    PrintAst(pParse,pSeq);
//    printf("pSeq->type %d\n",pSeq->type);
//    assert(pSeq->type==PROP_IMPL);


    TokenInfo *pR;
    TokenInfo *apCopy[5] = {0};

    if(pSeq->type==PROP_SYMB){
        return pSeq;
    }
    assert(pSeq->type==PROP_IMPL);
    if(pSeq->op==OP_ADD)
    {
        pR = PropAdd(pParse,ppTest,pSeq);
    }
    else
    {
        apCopy[0] = PropRemoveAdd(pParse,ppTest,pSeq->pLeft);
        apCopy[1] = PropRemoveAdd(pParse,ppTest,pSeq->pRight);
        pR = NewImplyNode(pParse,apCopy[0],apCopy[1],pSeq->zSymb);
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

int isConflictProp(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB)
{
    int rc = 0;
    int nA = 0;
    int nB = 0;

    while(pA->type==PROP_NEG)
    {
        pA = pA->pLeft;
        nA++;
    }
    while(pB->type==PROP_NEG)
    {
        pB = pB->pLeft;
        nB++;
    }
    if(pA==pB && (nA&1)!=(nB&1)){
        rc = 1;
    }
    return rc;
}

//(~A->(A->B))
TokenInfo *CreateNA_AB(
        AstParse *pParse,
        TokenInfo **ppTest,
        AddSeq *pA,//~
        AddSeq *pB)
{
    TokenInfo *pR = NULL;
    TokenInfo *apCopy[5] = {0};
    apCopy[1] = NewNumNode(pParse,NA_AB);
    apCopy[0] = NewImplyNode(pParse,pA->pSeq,apCopy[1],">");
    pR =  NewImplyNode(pParse,pB->pSeq,apCopy[0],">");
    return pR;
}

void PrintGenInfo(AstParse *pParse,AddSeq **ppMid,int idx)
{
#ifdef GEN_DEBUG
    printf("idx %d\n",idx);
    PrintAst(pParse,ppMid[idx]->pNode);
    PrintAst(pParse,ppMid[idx]->pSeq);
#endif
}

int isSameNode(AstParse *pParse,TokenInfo *pA,TokenInfo *pB)
{
    int rc = 0;
    while(pA->type==PROP_NEG && pB->type==PROP_NEG){
        pA = pA->pLeft;
        pB = pB->pLeft;
    }
    if(pA==pB){
        rc = 1;
    }
    return rc;
}

//不考虑双重否定的情况
void FindNewMpSeq(AstParse *pParse,AddSeq **ppMid,int i,int j,int *idx)
{
    TokenInfo *pNoNeg;
    TokenInfo *apCopy[5] = {0};
    //if(ppMid[i]->pNode==ppMid[j]->pNode->pLeft)
    if(isSameNode(pParse,ppMid[i]->pNode,ppMid[j]->pNode->pLeft))
    {
        ppMid[*idx]->pNode = ppMid[j]->pNode->pRight;
        ppMid[*idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,ppMid[j]->pSeq,">");
        PrintGenInfo(pParse,ppMid,*idx);
        (*idx)++;
    }
    else if(ppMid[j]->pNode->pRight->type==PROP_NEG)
    {
        if(ppMid[i]->pNode==ppMid[j]->pNode->pRight->pLeft){
            if(ppMid[j]->pNode->pLeft->type==PROP_NEG)
            {
                pNoNeg = ppMid[j]->pNode->pLeft->pLeft;
                apCopy[0] = NewImplyNode(pParse,ppMid[j]->pSeq,pParse->apAxiom[2],">");
                ppMid[*idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[0],">");
            }
            else
            {
                pNoNeg = NewNegNode(pParse,ppMid[j]->pNode->pLeft);
                apCopy[1] = NewNumNode(pParse,NNA_A);
                apCopy[0] = NewImplyNode(pParse,apCopy[1],ppMid[j]->pSeq,">>");
                apCopy[2] = NewImplyNode(pParse,apCopy[0],pParse->apAxiom[2],">");
                ppMid[*idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[2],">");
            }
            ppMid[*idx]->pNode = pNoNeg;
            (*idx)++;
        }
    }
    else if(ppMid[i]->pNode->type==PROP_NEG)
    {
        if(ppMid[i]->pNode->pLeft==ppMid[j]->pNode->pRight)
        {
            apCopy[1] = NewNumNode(pParse,AB_NBNA);
            apCopy[0] = NewImplyNode(pParse,ppMid[j]->pSeq,apCopy[1],">");
            ppMid[*idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[0],">");
            ppMid[*idx]->pNode = NewNegNode(pParse,ppMid[j]->pNode->pLeft);
            (*idx)++;
        }
    }
}

TokenInfo * PropGenNegSeq(
        AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pProp,
        u8 bNeg)
{
    TokenInfo *pR = NULL;
    TokenInfo *apCopy[10] = {0};
    TokenInfo *pNeg;
    TokenInfo *pLeft;
    TokenInfo *pRight;

    assert(pProp->type==PROP_IMPL);
    pLeft = pProp->pLeft;
    pRight = pProp->pRight;
    if(pRight->type==PROP_NEG){
        pNeg = pRight->pLeft;
    }
    else
    {
        pNeg = NewNegNode(pParse,pRight);
    }
    if(pLeft->type!=PROP_NEG){
        apCopy[0] = NewNegNode(pParse,pLeft);
        apCopy[1] = NewImplyNode(pParse,pNeg,apCopy[0],"->");
        apCopy[2] = PropGenSeq(pParse,ppTest,apCopy[1]);
        if(bNeg){
            apCopy[3] = NewNumNode(pParse,A_NB_B_NA);
        }
        else{
            apCopy[3] = pParse->apAxiom[2];
        }
        pR = NewImplyNode(pParse,apCopy[2],apCopy[3],">");
    }
    else
    {
        apCopy[0] = pLeft->pLeft;
        apCopy[1] = NewImplyNode(pParse,pNeg,apCopy[0],"->");
        apCopy[2] = PropGenSeq(pParse,ppTest,apCopy[1]);
        apCopy[3] = NewNumNode(pParse,AB_NBNA);
        if(!bNeg)//~A->B  (~B->A)->(~A->~~B)->~A->B
        {
            apCopy[4] = NewNumNode(pParse,NNA_A);
            pR = NewImplyNode(pParse,apCopy[2],apCopy[4],">>");
        }
        else{
            pR = NewImplyNode(pParse,apCopy[2],apCopy[3],">");
        }
    }
    return pR;
}

#define MID_NUM 100
TokenInfo * PropGenSeq(
        AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pProp)
{
    TokenInfo *pR = NULL;
    AddSeq apMidFomula[MID_NUM] = {0};
   // AddSeq **ppMid = (AddSeq **)malloc(sizeof(AddSeq *)*MID_NUM);
    AddSeq *ppMid[MID_NUM];
    TokenInfo *apCopy[10] = {0};
    TokenInfo *pNoNeg;
    int idx = 0;
    int offset = 0;
    int max;
    int i,j;

    for(i=0;i<MID_NUM;i++)
    {
        ppMid[i] = &apMidFomula[i];
    }

    while(pProp->type==PROP_NEG){
        pNoNeg = pProp->pLeft;
        if(pNoNeg->type==PROP_NEG){
            pProp = pNoNeg->pLeft;
        }
        else
        {
            apCopy[0] = NewImplyNode(pParse,pNoNeg,pProp,"+");
            apCopy[1] = PropGenSeq(pParse,ppTest,apCopy[0]);
            apCopy[2] = NewNumNode(pParse,NNA_A);
            apCopy[3] = NewImplyNode(pParse,apCopy[2],apCopy[1],">>");
            apCopy[4] = NewNumNode(pParse,NA_A_A);
            pR = NewImplyNode(pParse,apCopy[3],apCopy[4],">");
            return pR;
        }
    }

    if(pProp->pRight->type!=PROP_NEG)
    {
        ppMid[0]->pNode = pProp->pLeft;
        ppMid[0]->pSeq = NewSymbNode(pParse,"A");
        ppMid[1]->pNode = NewNegNode(pParse,pProp->pRight);
        ppMid[1]->pSeq = NewSymbNode(pParse,"B");
        idx = 2;
        do{
          max = idx;
          for(i=offset;i<max;i++){
#ifdef GEN_DEBUG
              printf("i %d\n",i);
              PrintAst(pParse,ppMid[i]->pNode);
#endif
              for(j=0;j<max;j++){
#ifdef GEN_DEBUG
                  printf("i %d j %d\n",i,j);
                  PrintAst(pParse,ppMid[j]->pNode);
#endif
//                  static int jj = 0;
//                  jj++;
//                  if(jj==93)
//                      printf("jj %d\n",jj);

                  if(isConflictProp(pParse,ppMid[i]->pNode,ppMid[j]->pNode))
                  {
#ifdef GEN_DEBUG
                      printf("conflict %d %d\n",i,j);
                      PrintAst(pParse,ppMid[i]->pNode);
                      PrintAst(pParse,ppMid[j]->pNode);
#endif
                      if(ppMid[i]->pNode->type==PROP_NEG){
                          apCopy[0] = CreateNA_AB(pParse,ppTest,ppMid[i],ppMid[j]);
                      }
                      else{
                          apCopy[0] = CreateNA_AB(pParse,ppTest,ppMid[j],ppMid[i]);
                      }
                      apCopy[1] = NewImplyNode(pParse,ppMid[1]->pSeq,apCopy[0],"+");
                      apCopy[3] = NewNumNode(pParse,NA_A_A);
                      apCopy[2] = NewImplyNode(pParse,apCopy[1],apCopy[3],">");
                      pR = NewImplyNode(pParse,ppMid[0]->pSeq,apCopy[2],"+");
                      //PrintAst(pParse,pR);
                      return pR;
                  }
                  if(ppMid[j]->pNode->type==PROP_IMPL )
                  {
                      FindNewMpSeq(pParse,ppMid,i,j,&idx);
                  }
                  if(ppMid[i]->pNode->type==PROP_IMPL)
                  {
                      FindNewMpSeq(pParse,ppMid,j,i,&idx);
                  }
              }

              if(ppMid[i]->pNode->type==PROP_NEG)
              {
                  pNoNeg = ppMid[i]->pNode->pLeft;
                  if(pNoNeg->type==PROP_IMPL){
                      ppMid[idx]->pNode = pNoNeg->pLeft;
                      apCopy[0] = NewNumNode(pParse,N_AB_A);
                      ppMid[idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[0],">");
                      PrintGenInfo(pParse,ppMid,idx);
                      idx++;
                      ppMid[idx]->pNode = NewNegNode(pParse,pNoNeg->pRight);
                      apCopy[1] = NewNumNode(pParse,N_AB_NB);
                      ppMid[idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[1],">");
                      PrintGenInfo(pParse,ppMid,idx);
                      idx++;
                  }
                  else if(pNoNeg->type==PROP_NEG)
                  {
                      ppMid[idx]->pNode = pNoNeg->pLeft;
                      apCopy[0] = NewNumNode(pParse,NNA_A);
                      ppMid[idx]->pSeq = NewImplyNode(pParse,ppMid[i]->pSeq,apCopy[0],">");
                      PrintGenInfo(pParse,ppMid,idx);
                      idx++;
                  }

              }
          }
          offset = max;
        }while(offset<idx);
        if(pProp->pLeft->type!=PROP_SYMB)
        {
            pR = PropGenNegSeq(pParse,ppTest,pProp,0);
        }
    }
    else
    {
        pNoNeg = pProp->pRight->pLeft;
        if(pNoNeg->type==PROP_NEG)
        {
            //A->~~B
            apCopy[3] = NewImplyNode(pParse,pProp->pLeft,pNoNeg->pLeft,"->");
            apCopy[0] = PropGenSeq(pParse,ppTest,apCopy[3]);
            apCopy[1] = NewNumNode(pParse,A_NNA);
            pR = NewImplyNode(pParse,apCopy[0],apCopy[1],">>");
            //(A+B)>>L = A+(B>L);  下面这种方法无法保证apCopy[0]是A+B的形式
           // assert(apCopy[0]->op==OP_ADD);
//            apCopy[2] = NewImplyNode(pParse,apCopy[0]->pRight,apCopy[1],">");
//            pR = NewImplyNode(pParse,apCopy[0]->pLeft,apCopy[2],"+");
            return pR;
        }

        if(pNoNeg->type!=PROP_SYMB)
        {
            assert(pNoNeg->type==PROP_IMPL);//A->~(B->C)

            apCopy[0] = NewImplyNode(pParse,pProp->pLeft,pNoNeg->pLeft,"->");
#ifdef GEN_DEBUG
            printf("left\n");
            PrintAst(pParse,apCopy[0]);
#endif
            apCopy[1] = PropGenSeq(pParse,ppTest,apCopy[0]);//A->B

            apCopy[3] = NewNegNode(pParse,pNoNeg->pRight);
            apCopy[2] = NewImplyNode(pParse,pProp->pLeft,apCopy[3],"->");
#ifdef GEN_DEBUG
            printf("right\n");
            PrintAst(pParse,apCopy[2]);
#endif
            apCopy[4] = PropGenSeq(pParse,ppTest,apCopy[2]);//A->~C

            apCopy[5] = NewNumNode(pParse,A_NB_NAB);//B->(~C->~(B->C))
            apCopy[6] = NewImplyNode(pParse,apCopy[5],pParse->apAxiom[0],">");
            apCopy[7] = NewImplyNode(pParse,apCopy[6],pParse->apAxiom[1],">");
            apCopy[8] = NewImplyNode(pParse,apCopy[1],apCopy[7],">");

            apCopy[0] = NewImplyNode(pParse,apCopy[8],pParse->apAxiom[1],">");
            pR =  NewImplyNode(pParse,apCopy[4],apCopy[0],">");
        }
        else if(pProp->pLeft->type!=PROP_SYMB)//(A->B)->~C
        {
            pR = PropGenNegSeq(pParse,ppTest,pProp,1);
        }
        else {
            printf("can\'t theorem\n");
            PrintAst(pParse,pProp);
        }
    }
    assert(pR!=NULL);
    return pR;
}
