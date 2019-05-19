/*
 * mapping.c
 *
 *  Created on: May 15, 2019
 *      Author: Administrator
 */

#include"algebra.h"
#include "field.h"
#include "group.h"
#include "linear.h"
#include "poly.h"

void FieldMapping(MapEle* pMap, VectorEle* pVec)
{
    int i,j;
    for(i=0;i<pMap->nSigma;i++)
    {
        pMap->xSigma(pVec);
    }
    for(i=0;i<pMap->nTau;i++)
    {
        pMap->xTau(pVec);
    }
}
//
//MapEle* Isomorphisms(
//        OperateSys *pGroup,
//        MapEle* pEle1,
//        MapEle* pEle2,
//        int num)
//{
//    MapEle* pMap1;
//    MapEle* pMap2;
//    MapEle* pIsom;
//    pMap1 = FieldMapping(pEle1,num);
//    pMap2 = FieldMapping(pEle2,num);
//
//}
//
////E.Artin Galois THEOREM 12
//int CharacterSumZero(
//        FieldSys *pField,
//        MapEle* ppEle,
//        MapEle* pEle,
//        int num)
//{
//    //a1¦Ò1(x)+a2¦Ò2(x)...
//    return 0;
//}


//(2)^(1/4) ->i(2)^(1/4)
void SigmaMap(VectorEle* pSrc)
{
    FieldSys *pSub;
    OperateSys *pPlus;
    VectorEle* pTemp;
    VectorEle *paVec[2];
    paVec[0] = pSrc->aVecEle[0];
    paVec[1] = pSrc->aVecEle[1];
    pSub = paVec[0]->pSubField;
    pPlus = pSub->pGroup1;

    pTemp = paVec[0]->aVecEle[1];
    paVec[0]->aVecEle[1] = pPlus->xInvEle(paVec[1]->aVecEle[1]);
    FreeGroupEle(pPlus,paVec[1]->aVecEle[1]);
    paVec[1]->aVecEle[1] = pTemp;

    pTemp = paVec[0]->aVecEle[2];
    paVec[0]->aVecEle[2] = pPlus->xInvEle(pTemp);
    FreeGroupEle(pPlus,pTemp);
    pTemp = paVec[1]->aVecEle[2];
    paVec[1]->aVecEle[2] = pPlus->xInvEle(pTemp);
    FreeGroupEle(pPlus,pTemp);

    pTemp = paVec[1]->aVecEle[3];
    paVec[1]->aVecEle[3] = pPlus->xInvEle(paVec[0]->aVecEle[3]);
    FreeGroupEle(pPlus,paVec[0]->aVecEle[3]);
    paVec[0]->aVecEle[3] = pTemp;
}

//i->-i
void TauMap(VectorEle* pSrc)
{
    int i;
    FieldSys *pSub;
    OperateSys *pPlus;
    VectorEle* pTemp;
    VectorEle *paVec[2];
    paVec[0] = pSrc->aVecEle[0];
    paVec[1] = pSrc->aVecEle[1];
    pSub = paVec[0]->pSubField;
    pPlus = pSub->pGroup1;

    for(i=0; i<4; i++)
    {
        pTemp = paVec[1]->aVecEle[i];
        paVec[1]->aVecEle[i] = pPlus->xInvEle(pTemp);
        FreeGroupEle(pPlus,pTemp);
    }

}

MapEle *NewMapEle(MapEle *pSrc)
{
    MapEle *pGen;
    pGen = (MapEle *)malloc(sizeof(MapEle));
    memset(pGen,0,sizeof(MapEle));

    pGen->xSigma = pSrc->xSigma;
    pGen->xTau = pSrc->xTau;

    return pGen;
}
MapEle *AutomorphismsGen(OperateSys *pOpSys,u32 num)
{
    MapEle *pGen;

    pGen = NewMapEle(pOpSys->pBaseEle);
    num = num%8;
    pGen->nSigma = num%4;
    pGen->nTau = num/4;


    return pGen;
}


MapEle *AutomorphismsMult(MapEle *p1,MapEle *p2)
{
    MapEle *p;

    p = NewMapEle(p1);
    p->nSigma = (p1->nSigma+p2->nSigma)%4;
    p->nTau = (p1->nTau+p2->nTau)%2;

    return p;
}

int AutomorphismsEqual(MapEle *p1,MapEle *p2)
{
    int rc = 0;

    if(p1->nSigma==p2->nSigma && p1->nTau==p2->nTau )
    {
        rc = 1;
    }

    return rc;
}

MapEle *AutomorphismsInv(MapEle *p1)
{
    MapEle *p;

    p = NewMapEle(p1);
    p->nSigma = (4-p1->nSigma)%4;
    p->nTau = (2-p1->nTau+2)%2;

    return p;
}

//ÒÔx^4-2=0ÎªÀý
OperateSys *AutomorphismsObj(void)
{
    OperateSys *pGroup;
    MapEle *pBaseItem;

    pGroup = (OperateSys *)malloc(sizeof(OperateSys));
    memset(pGroup,0,sizeof(OperateSys));

    pBaseItem = (MapEle *)malloc(sizeof(MapEle));
    memset(pBaseItem,0,sizeof(MapEle));
    pBaseItem->xSigma = SigmaMap;
    pBaseItem->xTau = TauMap;
    pBaseItem->nSigma = 0;
    pBaseItem->nTau = 0;

    pGroup->nPara = 1;
    pGroup->pBaseEle = pBaseItem;
    pGroup->xGen = (void*)AutomorphismsGen;
    pGroup->xOperat = (void*)AutomorphismsMult;
    pGroup->xInvEle = (void*)AutomorphismsInv;
    pGroup->xIsEqual = (void*)AutomorphismsEqual;

    return pGroup;
}

int ExtendFieldTest(FieldSys *pField)
{
    int rc = 0;
    int i,j,k;
    VectorEle* pT[4];
    OperateSys *pOpSys = pField->pGroup2;
    OperateSys *pMap = AutomorphismsObj();
    MapEle *pEle;

    pEle = pMap->xGen(pMap,4);
    for(i=0; i<1; i++)
    {
        k = FakeRand(i);
        SetGenPara(pOpSys,k);
        pT[0] = pOpSys->xGen(pOpSys,k);
        pT[1] = pOpSys->xGen(pOpSys,k+2);
        pT[2] = pOpSys->xOperat(pT[0],pT[1]);

        for(j=0;j<3;j++)
        {
            PrintVal(pField,(VectorEle **)pT[j]->aVecEle,pT[j]->nEle);
        }
        for(j=0;j<3;j++)
        {
//            SigmaMap(pT[j]);
//            SigmaMap(pT[j]);
            FieldMapping(pEle,pT[j]);
        }
        pT[3] = pOpSys->xOperat(pT[0],pT[1]);

        for(j=0;j<4;j++)
        {
            PrintVal(pField,(VectorEle **)pT[j]->aVecEle,pT[j]->nEle);
        }
        for(j=0;j<4;j++)
        {
            FreeGroupEle(pOpSys,pT[j]);
        }


    }
    loga("Automorphisms:");
    IsGroup(pMap);

    return rc;
}
