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
#include "poly.h"

void SetVecEle(
        FieldSys *pField,
        VectorEle *p,
        int nEle,
        int *aCoef)
{
    OperateSys *pPlus = pField->pGroup1;
    u8 iGen;
    int i;

    for(i=0; i<nEle; i++)
    {
        if( aCoef[i]<0 )
        {
            iGen = -aCoef[i];
            pPlus->aGenPara[0] = 1;
        }
        else
        {
            pPlus->aGenPara[0] = 0;
            iGen = aCoef[i];
        }
        p->aVecEle[i] = pPlus->xGen(pPlus,iGen);
    }
}


VectorEle *PolyGen(OperateSys *pOpSys,u32 num)
{
    VectorEle *p;
    OperateSys *pPlus = pOpSys->pSubFiled->pGroup1;
    OperateSys *pMult = pOpSys->pSubFiled->pGroup2;
    int i,k=0;

    p = NewVector(pOpSys->nPara);
    for(i=0; i<p->nEle; i++)
    {
        p->aVecEle[i] = GenSubFieldEle(pPlus,num+i+k+10);

    }
    p->pSubField = pOpSys->pSubFiled;
    p->pPoly = ((VectorEle *)pOpSys->pBaseEle)->pPoly;
    p->eType = p->pPoly->eType;

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
    pV[1] = VectorMod(pField,pV[0],p1->pPoly,0);

//    if( pField->pGroup1->typeEle>0 )
//    {
//        PrintVal(pField,(VectorEle **)p2->aVecEle,p2->nEle);
//        PrintVal(pField,(VectorEle **)p1->aVecEle,p1->nEle);
//        PrintVal(pField,(VectorEle **)pV[0]->aVecEle,pV[0]->nEle);
//        PrintVal(pField,(VectorEle **)pV[1]->aVecEle,pV[1]->nEle);
//    }

//    PrintVal(pField,(VectorEle **)&p1,1);
//    PrintVal(pField,(VectorEle **)&p2,1);
//    PrintVal(pField,(VectorEle **)&pV[1],1);
//    sleep(1);
//    assert(0);

    FreeVector(pV[0]);

    return pV[1];
}

VectorEle *PolyPlus(VectorEle *p1,VectorEle *p2)
{
    FieldSys *pField = p1->pSubField;
    VectorEle *p;
    p = VectorPlus(pField,p1,p2);

    return p;
}

VectorEle *PolyPlusInv(VectorEle *p1)
{
    FieldSys *pField = p1->pSubField;
    OperateSys *pPlus = pField->pGroup1;
    VectorEle *p;
    int i;

    p = NewVector1(p1,p1->nEle);

    for(i=0; i<p1->nEle; i++)
    {
        p->aVecEle[i] = pPlus->xInvEle(p1->aVecEle[i]);
    }

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
    p->eType = pPoly->eType+1;
    p->pSubField = NULL;
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


//    int aSet1[3] = {-1,1,0} ;
//    SetVecEle(pField,p1,3,aSet1);

    pV[0] = CreatUnkownPoly(pField,p1->pPoly);
    pV[1] = VectorMult(pField,p1,pV[0]);
    pV[2] = VectorMod(pField,pV[1],p1->pPoly,0);
    pV[3] = VectorTran(pField,pV[2]);

//    if( pField->pGroup1->typeEle==0 )
//    {
//        PrintVal(pField,(VectorEle **)&p1,1);
//        PrintVal(pField,(VectorEle **)&p1->pPoly,1);
//        for(j=0; j<4; j++)
//        {
//            loga("j %d type %d",j,pV[0]->eType);
//            PrintVal(pField,(VectorEle **)pV[j]->aVecEle,
//                    pV[j]->nEle);
//        }
//    }

    pRight = NewVector1(p1,p1->nEle);
    pRight->aVecEle[0] = pPlus->xOperat(pMult->pBaseEle,pPlus->pBaseEle);
    for(i=1;i<p1->nEle;i++)
    {
        pRight->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
    }

//    int aSet2[3] = {1,1,-2} ;
//    SetVecEle(pField,pRight,3,aSet2);
    // loga("right type %d",pRight->eType);
//     loga("pRight");
//     PrintVal(pField,(VectorEle **)&pRight,1);

//    if( pField->pGroup1->typeEle>0 )
//    {
//        PrintVal(pField,(VectorEle **)p1->aVecEle,p1->nEle);
//        PrintVal(pField,(VectorEle **)p1->pPoly->aVecEle,p1->pPoly->nEle);
//
//        VectorEle **paVec = (VectorEle **)pV[0]->aVecEle;
//        for(j=0;j<pV[0]->nEle; j++)
//        {
//            PrintVal(pField,((VectorEle *)paVec[j])->aVecEle,
//                    ((VectorEle *)paVec[j])->nEle);
//        }
//
//
//        paVec = (VectorEle **)pV[3]->aVecEle;
//        for(j=0;j<pV[3]->nEle; j++)
//        {
//            PrintVal(pField,((VectorEle *)paVec[j])->aVecEle,
//                    ((VectorEle *)paVec[j])->nEle);
//        }
//        PrintVal(pField,(VectorEle **)pRight->aVecEle,pRight->nEle);
//    }

    p = NewVector1(p1,p1->nEle);
    rc = SolveLinearEqu(pField,(VectorEle **)pV[3]->aVecEle,pRight,p,
            0,p1->nEle,0,p1->nEle);

    for(i=0;i<4;i++)
    {
        FreeVector(pV[i]);
    }
    FreeVector(pRight);

    assert(rc);

//    loga("p");
//    PrintVal(pField,(VectorEle **)&p,1);
//    loga("");

//    sleep(1);
//    assert(0);

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
    VectorEle *pBaseItem;
    pBaseItem = (VectorEle*)malloc(sizeof(VectorEle));

    pBaseItem->nEle = nEle;
    pBaseItem->pSubField = pField;
    pBaseItem->pPoly = pPoly;
    paVecEle = (VectorEle **)malloc(nEle*sizeof(VectorEle *));
    for(i=0;i<nEle;i++)
    {
        paVecEle[i] = pPlus->xOperat(
                        pPlus->pBaseEle,
                        pPlus->pBaseEle);
    }
    pBaseItem->aVecEle = (void**)paVecEle;
    pBaseItem->eType = pPoly->eType;

    OperateSys *plus;
    plus = (OperateSys *)malloc(sizeof(OperateSys));
    memset(plus,0,sizeof(OperateSys));

    plus->pBaseEle = pBaseItem;
    plus->nPara = nEle;
    plus->typeEle = VEC_ELE;
    plus->xIsEqual = (void*)PolyEqual;
    plus->xGen = (void*)PolyGen;
    plus->xInvEle = (void*)PolyPlusInv;
    plus->xOperat =  (void*)PolyPlus;
    plus->pSubFiled = pField;

    return plus;
}

void *CalPolyVal(
        FieldSys *pField,
        VectorEle *pPoly,
        void *p1
        )
{
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pPlus = pField->pGroup1;
    int i,j;
    void *pExp;
    void *pSum;
    void *pT[5];

    pSum = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
    for(i=0;i<pPoly->nEle;i++)
    {
        pExp = pPlus->xOperat(pPlus->pBaseEle,pMult->pBaseEle);
        for(j=0; j<i; j++)
        {
            pT[0] = pExp;
            pExp = pMult->xOperat(pExp,p1);
            FreeGroupEle(pMult,pT[0]);
        }
        pT[0] = pExp;
        pExp = FieldMultVector(pPoly->pSubField,pPoly->aVecEle[i],pExp);
        FreeGroupEle(pMult,pT[0]);
        pT[0] = pSum;
        pSum = pPlus->xOperat(pSum,pExp);
        FreeGroupEle(pMult,pT[0]);
    }

    return pSum;
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
    VectorEle *pBaseItem;

    pBaseItem = (VectorEle *)malloc(sizeof(VectorEle));
    pBaseItem->nEle = nEle;
    pBaseItem->pSubField = pField;
    pBaseItem->pPoly = pPoly;
    pBaseItem->eType = pPoly->eType;

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
    pBaseItem->aVecEle = (void**)paVecEle;


    OperateSys *mult;
    mult = (OperateSys *)malloc(sizeof(OperateSys));
    memset(mult,0,sizeof(OperateSys));

    mult->pBaseEle = pBaseItem;
    mult->nPara = nEle;
    mult->isMult = 1;
    mult->typeEle = VEC_ELE;
    mult->xIsEqual = (void*)PolyEqual;
    mult->xGen = (void*)PolyGen;
    mult->xInvEle = (void*)PolyMultInv;
    mult->xOperat =  (void*)PolyMult;
    mult->pSubFiled = pField;

    return mult;
}

void *GenVecEle(FieldSys *pField, int num)
{
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    void *pVec;
    void *pTemp;
    int i;
    pVec = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
    for(i=0;i<num;i++)
    {
        pTemp = pVec;
        pVec = pPlus->xOperat(pVec,pMult->pBaseEle);
        FreeGroupEle(pPlus,pTemp);
    }

    return pVec;

}
//x-α
VectorEle *NewLinerPoly(FieldSys *pField,VectorEle *pPoly)
{
    VectorEle *p;
    VectorEle *pBase = pField->pGroup1->pBaseEle;
    VectorEle *pIdentity = pField->pGroup2->pBaseEle;
    VectorEle *pVec;
    OperateSys *pPlus = pBase->pSubField->pGroup1;
    OperateSys *pMult = pBase->pSubField->pGroup2;
    int i,j;

    assert( pPoly->nEle>2 );
    p = NewVector(pPoly->nEle-1);
    p->pSubField = pField;
    p->eType = pPoly->eType;
    p->pPoly = pPoly;

    p->aVecEle[0] = NewVector1(pBase,pBase->nEle);
    pVec = p->aVecEle[0];
    for(j=0;j<pBase->nEle;j++)
    {
        if( j==1 )
        {
            pVec->aVecEle[j] = pPlus->xInvEle(pMult->pBaseEle);
        }
        else
        {
            pVec->aVecEle[j] = pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
        }
    }
    p->aVecEle[1] = pField->pGroup1->xOperat(pBase,pIdentity);
    for(i=2; i<pPoly->nEle-1; i++)
    {
        p->aVecEle[i] = pField->pGroup1->xOperat(pBase,pBase);
    }

    return p;
}


//根据参数来设置扩域后的多项式
VectorEle *NewPolyVecPara(
        FieldSys *pField,
        VectorEle *pPoly,
        int (*aPara)[3])
{
    VectorEle *p;
    VectorEle *pBase = pField->pGroup1->pBaseEle;
    VectorEle *pVec;
    VectorEle *pTemp;
    int i,j;

    assert( pBase->eType==pPoly->eType );

    p = NewVector(pPoly->nEle);
    p->pSubField = pField;
    p->eType = pPoly->eType+1;
    p->pPoly = NULL;

    for(i=0; i<pPoly->nEle; i++)
    {
        p->aVecEle[i] = NewVector1(pPoly,pBase->nEle);

        pTemp = NewVector1(pBase,pBase->nEle);
        SetVecEle(pBase->pSubField,pTemp,pTemp->nEle,aPara[i]);
        p->aVecEle[i] = pTemp;
            //FreeVector(pTemp);
    }

    return p;
}

//把多项式系数写成扩充的新域的形式
VectorEle *GetNewFieldPoly(FieldSys *pField,VectorEle *pPoly)
{
    OperateSys *pPlus = pPoly->pSubField->pGroup1;
    VectorEle *p;
    VectorEle *pZero = pPlus->pBaseEle;
    VectorEle *pBase = pField->pGroup1->pBaseEle;
    VectorEle *pVec;
    int i,j;

    assert( pBase->eType==pPoly->eType );

    p = NewVector(pPoly->nEle);
    p->pSubField = pField;
    p->eType = pPoly->eType+1;
    p->pPoly = NULL;

    for(i=0; i<pPoly->nEle; i++)
    {
        p->aVecEle[i] = NewVector1(pBase,pBase->nEle);
        pVec =  p->aVecEle[i];
        pVec->aVecEle[0] = pPlus->xOperat(pZero,pPoly->aVecEle[i]);
        for(j=1; j<pBase->nEle; j++)
        {
            pVec->aVecEle[j] = pPlus->xOperat(pZero,pZero);
        }
    }

    return p;
}

VectorEle *NewPolyVec(FieldSys *pField,int *aCoef, int nEle)
{
    OperateSys *pPlus = pField->pGroup1;
    VectorEle *p;
    u8 iGen;
    int i;

    //如果多项式是x^3+1=0
    //那么向量空间的最大次数就是x^2
    //所以多项式的次数比向量空间的次数大1
    p = NewVector(nEle+1);
    p->eType = BASE_ELE_TYPE;
    p->pPoly = NULL;
    p->pSubField = pField;
    for(i=0; i<nEle; i++)
    {
        if( aCoef[i]<0 )
        {
            iGen = -aCoef[i];
            pPlus->aGenPara[0] = 1;
        }
        else
        {
            pPlus->aGenPara[0] = 0;
            iGen = aCoef[i];
        }
        p->aVecEle[i] = pPlus->xGen(pPlus,iGen);
    }
    pPlus->aGenPara[0] = 0;
    p->aVecEle[i] = pPlus->xGen(pPlus,1);

    return p;
}

void SetVecField(
        FieldSys **ppField,
        FieldSys *pSubField,
        int nEle)
{
    int aCoef[10] = {0};
    VectorEle *pPoly;
    *ppField = (FieldSys *)malloc(sizeof(FieldSys));

#if 0
    aCoef[0] = 3;
    aCoef[1] = 1;
    aCoef[2] = 1;
    aCoef[3] = 1;
    aCoef[4] = 1;
    aCoef[5] = 1;
#else
    aCoef[0] = -4;
    aCoef[1] = 0;
    aCoef[2] = 0;
    aCoef[3] = 0;
    aCoef[4] = 1;
    aCoef[5] = 0;
#endif
    pPoly = NewPolyVec(pSubField,aCoef,nEle);

    (*ppField)->pGroup1 = PolyPlusObj(pSubField,nEle,pPoly);
    (*ppField)->pGroup2 = PolyMultObj(pSubField,nEle,pPoly);
    (*ppField)->pSub = pSubField;
    pSubField->pParent = (*ppField);

}

FieldSys *SplittingField(FieldSys *pField)
{
    OperateSys *pPlus = pField->pGroup1;
    FieldSys *pExtend;
    VectorEle *pPoly;
    VectorEle *pDiv;
    VectorEle *pTemp;

    pExtend = (FieldSys *)malloc(sizeof(FieldSys));

    pTemp = pPlus->pBaseEle;
    pPoly = GetNewFieldPoly(pField,pTemp->pPoly);
    PrintVec(pField,pPoly);
    pDiv = NewLinerPoly(pField,pPoly);
    PrintVec(pField,pDiv);
    pTemp = pPoly;
    pPoly = VectorMod(pField,pPoly,pDiv,1);
    FreeVector(pTemp);
    PrintVec(pField,pPoly);

    pExtend->pGroup1 = PolyPlusObj(pField,pPoly->nEle-1,pPoly);
    pExtend->pGroup2 = PolyMultObj(pField,pPoly->nEle-1,pPoly);
    pField->pParent = pExtend;
    loga("Extend %d",pPoly->nEle);
    IsField(pExtend);

    return pExtend;
}

FieldSys *ExtendField(FieldSys *pField)
{
    FieldSys *pSub = pField->pSub;
    FieldSys *pExtend;
    VectorEle *pPoly;
    VectorEle *pTemp;
    int aCoef[10] = {1,0,1};

    pExtend = (FieldSys *)malloc(sizeof(FieldSys));

    pTemp = NewPolyVec(pSub,aCoef,2);
    pPoly = GetNewFieldPoly(pField,pTemp);
    FreeVector(pTemp);
    PrintVec(pField,pPoly);

    pExtend->pGroup1 = PolyPlusObj(pField,pPoly->nEle-1,pPoly);
    pExtend->pGroup2 = PolyMultObj(pField,pPoly->nEle-1,pPoly);
    pField->pParent = pExtend;
    loga("Extend %d",pPoly->nEle);
    IsField(pExtend);

    return pExtend;
}



void PolyTest(FieldSys *pField)
{
    int rc = 0;
    VectorEle *pTest;
    VectorEle *pRslt;
    VectorEle *pNewPoly;
    VectorEle *pBase = pField->pGroup1->pBaseEle;
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pPlus = pField->pGroup1;
    FieldSys *pExtend;

    loga("poly");
    IsField(pField);

#if 0
    pTest = NewVector1(pBase,pBase->nEle);
    //int aSet1[5] = {-1,1,0} ;
    int aSet2[5] = {59,-6,9,-60,83};
    SetVecEle(pBase->pSubField,pTest,5,aSet2);
  //  pRslt = VectorMod(pBase->pSubField,pBase->pPoly,pTest,0);
    pRslt = pMult->xInvEle(pTest);
    PrintVal(pField,(VectorEle **)&pRslt,1);
#endif

    pNewPoly = GetNewFieldPoly(pField,pBase->pPoly);
    loga("--");
    PrintVal(pField,(VectorEle **)pNewPoly->aVecEle,pNewPoly->nEle);
    //pTest = NewPolyVecPara(pField,pTest,aPara);
    pTest = NewLinerPoly(pField,pNewPoly);
    loga("--");
    PrintVal(pField,(VectorEle **)pTest->aVecEle,pTest->nEle);
    loga("--");
    pRslt = VectorMod(pField,pNewPoly,pTest,1);
    //pRslt = CalPolyVal(pField,pBase->pPoly,pTest);
    loga("r--");
    PrintVal(pField,(VectorEle **)pRslt->aVecEle,pRslt->nEle);
    PrintVal(pField,(VectorEle **)&pBase->pPoly,1);
    rc = pField->pGroup1->xIsEqual(pBase,pRslt);
    assert(rc);
    pExtend = ExtendField(pField);

    ExtendFieldTest(pExtend);

//    loga("splite");
    //pExtend = SplittingField(pField);
//    pExtend = SplittingField(pExtend);
//    pExtend = SplittingField(pExtend);

}
