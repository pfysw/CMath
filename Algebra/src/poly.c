/*
 * polynomial.c
 *
 *  Created on: Apr 27, 2019
 *      Author: Administrator
 */

#include"algebra.h"
#include "field.h"
#include "group.h"
#include "linear.h"


VectorEle *PolyGen(OperateSys *pOpSys,u32 num)
{
    VectorEle *p;
    OperateSys *pPlus = pOpSys->pSubFiled->pGroup1;
    OperateSys *pMult = pOpSys->pSubFiled->pGroup2;
    int i,k=0;

    p = NewVector(pOpSys->nPara);
    for(i=0; i<p->nEle; i++)
    {
        p->aVecEle[i] = GenRatuonEle(pPlus,num+i+k+10);

    }
    p->pSubField = pOpSys->pSubFiled;
    p->pPoly = ((VectorEle *)pOpSys->pBaseEle)->pPoly;

    return p;
}

int PolyEqual(VectorEle *p1,VectorEle *p2)
{
    int rc;
    FieldSys *pField = p1->pSubField;
    rc = isVecotrEqual(pField,p1,p2);

    return rc;
}
VectorEle *PolyMult(VectorEle *p1,VectorEle *p2)
{
    VectorEle *pV[2];
    FieldSys *pField = p1->pSubField;

    pV[0] = VectorMult(pField,p1,p2);

    FieldEle **paEle;
    paEle = (FieldEle **)pV[0]->aVecEle;

//    loga("VectorMult");
//    for(int ii=0; ii<pV[0]->nEle; ii++)
//    {
//        logc("%.2f  ",paEle[ii]->val);
//    }
//    logc("\n");

    pV[1] = VectorMod(pField,pV[0],p1->pPoly);
    pV[1]->pSubField = pField;
    pV[1]->pPoly = p1->pPoly;

    free(pV[0]);

    return pV[1];
}

VectorEle *PolyPlus(VectorEle *p1,VectorEle *p2)
{
    FieldSys *pField = p1->pSubField;
    VectorEle *p;
    p = VectorPlus(pField,p1,p2);
    p->pSubField = pField;
    p->pPoly = p1->pPoly;

    return p;
}

VectorEle *PolyPlusInv(VectorEle *p1)
{
    FieldSys *pField = p1->pSubField;
    OperateSys *pPlus = pField->pGroup1;
    VectorEle *p;
    int i;

    p = NewVector(p1->nEle);

    for(i=0; i<p1->nEle; i++)
    {
        p->aVecEle[i] = pPlus->xInvEle(p1->aVecEle[i]);
    }
    p->pSubField = pField;
    p->pPoly = p1->pPoly;

    return p;
}

VectorEle *CreatUnkownVec(FieldSys *pField,VectorEle *pPoly,int k)
{
    VectorEle *p;
    int nEle = pPoly->nEle;
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    int i;

    p = NewVector(nEle-1);
    p->eType = pPoly->eType;
    p->pSubField = pField;
    p->pPoly = pPoly;
    for(i=0; i<nEle-1; i++)
    {
        if( k!=i )
            p->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
        else
            p->aVecEle[i] = pPlus->xOperat(pMult->pBaseEle,pPlus->pBaseEle);
    }

    return p;
}

VectorEle *CreatUnkownPoly(FieldSys *pField,VectorEle *pPoly)
{
    VectorEle *p;
    int nEle = pPoly->nEle;
    int i;
    //p是一个由未知量系数组成的多项式
    //例如x0+x1*a+x2*a^2+....
    //这里x0，x1，x2...用向量来表示
    //最高项次数比多项式少1
    p = NewVector(nEle-1);
    p->eType = FIRST_LEVEL_VEC;
    p->pSubField = NULL;//lijia  待修改
    p->pPoly = NULL;
    for(i=0; i<nEle-1; i++)
    {
        p->aVecEle[i] = CreatUnkownVec(pField,pPoly,i);
    }

    return p;
}

VectorEle *PolyMultInv(VectorEle *p1)
{
    FieldSys *pField = p1->pSubField;
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    VectorEle *p;
    VectorEle *pV[4];
    VectorEle *pRight;
    int i,j;
    int rc;

//    PrintVal(pField,(VectorEle **)&p1,1);
//    PrintVal(pField,(VectorEle **)&p1->pPoly,1);

    pV[0] = CreatUnkownPoly(pField,p1->pPoly);
    pV[1] = VectorMult(pField,p1,pV[0]);
    pV[2] = VectorMod(pField,pV[1],p1->pPoly);
    pV[3] = VectorTran(pField,pV[2]);

    for(j=0; j<4; j++)
    {
        loga("j %d type %d",j,pV[0]->eType);
        PrintVal(pField,(VectorEle **)pV[j]->aVecEle,
                pV[j]->nEle);
    }
    pRight = NewVector1(p1,p1->nEle);
    pRight->aVecEle[0] = pPlus->xOperat(pMult->pBaseEle,pPlus->pBaseEle);
    for(i=1;i<p1->nEle;i++)
    {
        pRight->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
    }

//    loga("pRight");
//    PrintVal(pField,(VectorEle **)&pRight,1);


    p = NewVector1(p1,p1->nEle);
    rc = SolveLinearEqu(pField,(VectorEle **)pV[3]->aVecEle,pRight,p,
            0,p1->nEle,0,p1->nEle);

//    loga("p");
//    PrintVal(pField,(VectorEle **)&p,1);
    assert(rc);

    return p;
}

OperateSys *PolyPlusObj(
        FieldSys *pField,
        int nEle,
        VectorEle *pPoly)
{
    VectorEle **paVecEle;
    OperateSys *pPlus = pField->pGroup1;
    int i;
    static VectorEle baseItem;

    baseItem.nEle = nEle;
    baseItem.pSubField = pField;
    baseItem.pPoly = pPoly;
    paVecEle = (VectorEle **)malloc(nEle*sizeof(VectorEle *));
    for(i=0;i<nEle;i++)
    {
        paVecEle[i] = pPlus->xOperat(
                        pPlus->pBaseEle,
                        pPlus->pBaseEle);
    }
    baseItem.aVecEle = (void**)paVecEle;

    static OperateSys plus;
    memset(&plus,0,sizeof(plus));

    plus.pBaseEle = &baseItem;
    plus.nPara = nEle;
    plus.typeEle = VEC_ELE;
    plus.xIsEqual = (void*)PolyEqual;
    plus.xGen = (void*)PolyGen;
    plus.xInvEle = (void*)PolyPlusInv;
    plus.xOperat =  (void*)PolyPlus;
    plus.pSubFiled = pField;

    return &plus;
}

OperateSys *PolyMultObj(
        FieldSys *pField,
        int nEle,
        VectorEle *pPoly)
{
    VectorEle **paVecEle;
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pPlus = pField->pGroup1;
    int i;
    static VectorEle baseItem;

    baseItem.nEle = nEle;
    baseItem.pSubField = pField;
    baseItem.pPoly = pPoly;

    paVecEle = (VectorEle **)malloc(nEle*sizeof(VectorEle *));
    paVecEle[0] = pMult->xOperat(
                    pMult->pBaseEle,
                    pMult->pBaseEle);
    for(i=1;i<nEle;i++)
    {
        paVecEle[i] = pMult->xOperat(
                pMult->pBaseEle,
                        pPlus->pBaseEle);
    }
    baseItem.aVecEle = (void**)paVecEle;

    static OperateSys mult;
    memset(&mult,0,sizeof(mult));

    mult.pBaseEle = &baseItem;
    mult.nPara = nEle;
    mult.isMult = 1;
    mult.typeEle = VEC_ELE;
    mult.xIsEqual = (void*)PolyEqual;
    mult.xGen = (void*)PolyGen;
    mult.xInvEle = (void*)PolyMultInv;
    mult.xOperat =  (void*)PolyMult;
    mult.pSubFiled = pField;

    return &mult;
}

VectorEle *NewPolyVec(FieldSys *pField,int nEle)
{
    OperateSys *pPlus = pField->pGroup1;
    VectorEle *p;
    int i;
    u8 aCoef[10] = {0};
    aCoef[0] = 1;
    aCoef[5] = 1;
    //如果多项式是x^3+1=0
    //那么向量空间的最大次数就是x^2
    //所以多项式的次数比向量空间的次数大1
    p = NewVector(nEle+1);
    p->eType = BASE_ELE_TYPE;
    p->pPoly = NULL;
    p->pSubField = pField;
    for(i=0; i<nEle; i++)
    {
        p->aVecEle[i] = pPlus->xGen(pPlus,aCoef[i]);
    }
    p->aVecEle[i] = pPlus->xGen(pPlus,1);

    return p;
}

void SetVecField(
        FieldSys **ppField,
        FieldSys *pSubField,
        int nEle)
{
    VectorEle *pPoly;
    *ppField = (FieldSys *)malloc(sizeof(FieldSys));
    pPoly = NewPolyVec(pSubField,nEle);

    (*ppField)->pGroup1 = PolyPlusObj(pSubField,nEle,pPoly);
    (*ppField)->pGroup2 = PolyMultObj(pSubField,nEle,pPoly);

    pSubField->pParent = (*ppField);

}

void PolyTest(FieldSys *pField)
{
    loga("poly");
    IsGroup(pField->pGroup1);
    IsGroup(pField->pGroup2);
}
