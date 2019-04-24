/*
 * linear.c
 *
 *  Created on: Apr 23, 2019
 *      Author: Administrator
 */
#include"algebra.h"
#include "field.h"
#include "group.h"
#include "linear.h"


FieldEle *EliminationUnkowns(
        FieldSys *pField,
        FieldEle *pEle1,
        FieldEle *pEle2,
        FieldEle *pCoef)
{
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    FieldEle *pT[5];

    pT[0] = pMult->xOperat(pCoef,pEle2);
    pT[1] = pPlus->xInvEle(pT[0]);
    pT[2] = pPlus->xOperat(pEle1,pT[1]);
    free(pT[0]);
    free(pT[1]);
    free(pEle1);

    return pT[2];
}

FieldEle *FiledDiv(
        FieldSys *pField,
        FieldEle *pEle1,
        FieldEle *pEle2)
{
    OperateSys *pMult = pField->pGroup2;
    FieldEle *pT[2];

    pT[0] = pMult->xInvEle(pEle2);
    pT[1] = pMult->xOperat(pEle1,pT[0]);
    free(pT[0]);
    return pT[1];
}

void NewVector(FieldSys *pField)
{

    int i,j,k;
    VectorEle **paVector;
    FieldEle *pEle;
    pField->paVector = (VectorEle **)malloc(COL*sizeof(VectorEle *));
    paVector = pField->paVector;
    for(i=0; i<COL; i++)
    {
        paVector[i] = (VectorEle *)malloc(sizeof(VectorEle));
        paVector[i]->nEle = ROW;
        paVector[i]->aVecEle = malloc(ROW*sizeof(void *));
        for(j=0;j<ROW;j++)
        {
            k = FakeRand(i+j+5);
            SetGenPara(pField->pGroup2,k);
           // if(j==3)k=0;
            paVector[i]->aVecEle[j] = pField->pGroup2->xGen(pField->pGroup2,k);
            pEle = paVector[i]->aVecEle[j];
            char asym[2] = {'+','-'};
            logc("%c%d/%d ",asym[pEle->eSymb],pEle->nmrtr,pEle->dnmtr);
        }
        logc("\n");
    }

}
int isLinearDepedent(
        FieldSys *pField,
        VectorEle **paVector,
        int n,
        int iRow,
        int nRow)
{
    int rc = 0;
    int i,j;
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    FieldEle **paEle;
    FieldEle *pTemp;
    FieldEle *pTempEle;

    assert(n>0);
    assert(nRow>0);
    if( n==1 )
    {
        paEle= (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        for(i=0; i<nRow; i++)
        {
            //线性变换后，最后一列有非零元素，所以不可能存在非零解，线性无关
            if( !pPlus->xIsEqual(paEle[i],pPlus->pBaseEle) )
            {
                return 0;
            }
        }
        return 1;
    }
    else if( nRow==1 )
    {
        return 1;
    }
    else
    {
        paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        for(i=0; i<nRow; i++)
        {
            //找到第1列的非0元素，换到第一行
            if( !pPlus->xIsEqual(paEle[i],pPlus->pBaseEle) )
            {

                for(j=0;j<n;j++)
                {
                    paEle = (FieldEle **)&((paVector[j])->aVecEle[iRow]);
                    pTempEle = paEle[i];
                    paEle[i] = paEle[0];
                    paEle[0] = pTempEle;
                }
                break;
            }
        }
        paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        if( pPlus->xIsEqual(paEle[0],pPlus->pBaseEle) )
        {
            rc = isLinearDepedent(pField,&paVector[1],n-1,iRow,nRow);
        }
        else
        {
            for(i=1; i<nRow; i++)
            {
                paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
                pTemp = FiledDiv(pField,paEle[i],paEle[0]);
//                FieldEle *p1=NULL,*p2=NULL;
//                pTemp = FiledDiv(pField,p1,p2);
                for(j=0;j<n;j++)
                {
                    paEle = (FieldEle **)&((paVector[j])->aVecEle[iRow]);
                    paEle[i] = EliminationUnkowns(pField,paEle[i],paEle[0],pTemp);
                }
                free(pTemp);
            }
            rc = isLinearDepedent(pField,&paVector[1],n-1,iRow+1,nRow-1);
        }
    }

    return rc;
}
