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


VectorEle *NewVector(int nEle)
{
    VectorEle *pVector;
    int size;
    pVector = (VectorEle *)malloc(sizeof(VectorEle));
    memset(pVector,0,sizeof(VectorEle));
    pVector->nEle = nEle;
    size = nEle*sizeof(void *);
    pVector->aVecEle = malloc(size);
    memset(pVector->aVecEle,0,size);
    return pVector;
}

VectorEle *NewVector1(VectorEle *pSrc,int nEle)
{
    VectorEle *pVector;
    int size;
    pVector = (VectorEle *)malloc(sizeof(VectorEle));
    memset(pVector,0,sizeof(VectorEle));
    pVector->nEle = nEle;
    size = nEle*sizeof(void *);
    pVector->aVecEle = malloc(size);
    memset(pVector->aVecEle,0,size);
    pVector->pSubField = pSrc->pSubField;
    pVector->pPoly = pSrc->pPoly;
    pVector->eType = pSrc->eType;

    return pVector;
}


FieldEle *EliminationUnkowns(
        FieldSys *pField,
        FieldEle *pEle1,
        FieldEle *pEle2,
        FieldEle *pCoef,
        u8 flag)
{
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    void *pT[5];

    if( flag )
    {
        //这里使用向量加法
        pPlus = pField->pParent->pGroup1;
        //pEle2是一个数，pEle1和pCoef是一个向量
        pT[0] = FieldMultVector(pField,pEle2,(VectorEle *)pCoef);
        pT[1] = pPlus->xInvEle(pT[0]);
        pT[2] = pPlus->xOperat(pEle1,pT[1]);
    }
    else
    {
        pT[0] = pMult->xOperat(pCoef,pEle2);
        pT[1] = pPlus->xInvEle(pT[0]);
        pT[2] = pPlus->xOperat(pEle1,pT[1]);
    }
    FreeGroupEle(pPlus,pT[0]);
    FreeGroupEle(pPlus,pT[1]);
    FreeGroupEle(pPlus,pEle1);

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
    FreeGroupEle(pMult,pT[0]);
    return pT[1];
}

VectorEle *VectorDivField(
        FieldSys *pField,
        VectorEle *pVecEle,
        FieldEle *pEle)
{
    OperateSys *pMult = pField->pGroup2;
    int nEle = pVecEle->nEle;
    FieldEle *pT[2];
    VectorEle *pVec;
    int i;

    pVec = NewVector1(pVecEle,nEle);
    pT[0] = pMult->xInvEle(pEle);
    for(i=0;i<nEle;i++)
    {
        pVec->aVecEle[i] = pMult->xOperat(pVecEle->aVecEle[i],pT[0]);
    }
    FreeGroupEle(pMult,pT[0]);

    return pVec;
}

//float TestVal[3][5] =
//{
//        1,0,0,0,0,
//        2,1,0,0,0,
//        3,0,3,0,0,
//};



VectorEle *VectorPlus(
        FieldSys *pField,
        VectorEle *pVector1,
        VectorEle *pVector2)
{
    int nEle = pVector1->nEle;
    VectorEle *pVector;
    int i;

    pVector = NewVector1(pVector1,nEle);
    for(i=0;i<nEle;i++)
    {
        pVector->aVecEle[i] = pField->pGroup1->xOperat(
                pVector1->aVecEle[i],pVector2->aVecEle[i] );
    }

    return pVector;
}

VectorEle *VectorMult(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2)
{
    VectorEle *pVector;
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pPlus = pField->pGroup1;
    void *pT[2];
    int i,j,k;

    pVector = NewVector1(p2,p1->nEle+p2->nEle-1);

    for(i=0;i<p1->nEle;i++)
    {
        for(j=0;j<p2->nEle;j++)
        {
            //两个多项式中的元素两两相乘
            //次数相等的项全部加在一起
            k = i+j;
            if( p1->eType!=p2->eType )
            {
                assert( p1->eType<p2->eType );
                pT[0] = (void*)FieldMultVector(pField,p1->aVecEle[i],p2->aVecEle[j]);
            }
            else
            {
                pT[0] = pMult->xOperat(p1->aVecEle[i],p2->aVecEle[j]);
            }

            if( pVector->aVecEle[k]==NULL )
            {
                pVector->aVecEle[k] = pT[0];
            }
            else
            {
                if( p1->eType!=p2->eType )
                {
                    assert( p1->eType<p2->eType );
                    pT[1] = (void*)VectorPlus(pField,pT[0],pVector->aVecEle[k]);
                    FreeVector((VectorEle *)pVector->aVecEle[k]);
                    FreeVector((VectorEle *)pT[0]);
                }
                else
                {
                    pT[1] = pPlus->xOperat(pT[0],pVector->aVecEle[k]);
                    free(pVector->aVecEle[k]);
                    free(pT[0]);
                }
                pVector->aVecEle[k] = pT[1];
            }

        }
    }

    return pVector;
}

//矩阵转置
VectorEle *VectorTran(
        FieldSys *pField,
        VectorEle *p1)
{
    VectorEle *p;
    VectorEle *pCol;
    VectorEle **paRow;
    OperateSys *pPlus = pField->pGroup1;
    int i,j;

    assert( p1->eType>BASE_ELE_TYPE );

    p = NewVector1(p1,p1->nEle);

    //初始化矩阵空间
    for(i=0;i<p1->nEle;i++)
    {
        pCol = p1->aVecEle[i];
        assert( pCol->nEle==p1->nEle );
        p->aVecEle[i] = NewVector1(pCol,p1->nEle);
    }

    //p的第i列j行等于p1的第列i行
    for(i=0;i<p1->nEle;i++)
    {
        paRow = ((VectorEle *)p->aVecEle[i])->aVecEle;
        for(j=0;j<p1->nEle;j++)
        {
            pCol = p1->aVecEle[j];
            paRow[j] = pPlus->xOperat(pPlus->pBaseEle,
                    pCol->aVecEle[i]);
        }
    }
    return p;

}

VectorEle *VectorMod(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2,
        u8 bQuotient)//0：求余 1：求商
{
    VectorEle *pVector;
    VectorEle *pTemp;
    OperateSys *pPlus;
    //FieldEle *pT[2]; //调试用
    void *pT[2];
    int i,j,l;
    int mxExp;

    if( p1->eType!=p2->eType )
    {
        assert(p1->eType>p2->eType);
        pPlus = pField->pParent->pGroup1;
    }
    else
    {
        pPlus = pField->pGroup1;
    }

    pTemp = NewVector1(p1,p1->nEle);
    for(i=0; i<p1->nEle; i++)
    {
        pTemp->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,p1->aVecEle[i]);
    }

    for(i=p2->nEle-1; i>0; i--)
    {
        if( !pField->pGroup1->xIsEqual(
                pField->pGroup1->pBaseEle,p2->aVecEle[i]) )
        {
            mxExp = i;
            break;
        }
    }

    //loga("nEle %d %d",p1->nEle,p2->nEle);
    if( p1->nEle>=mxExp+1 )
    {
        if( bQuotient )
        {
            pVector = NewVector1(p1,p1->nEle-mxExp);
        }
        else
        {
            pVector = NewVector1(p1,mxExp);
        }

        for(i=p1->nEle-1;i>=mxExp;i--)
        {
            if( pTemp->eType==p2->eType )
            {
                //除式最高次项系数与pTemp元的比值
                pT[0] = FiledDiv(pField,pTemp->aVecEle[i],
                        p2->aVecEle[mxExp]);

                for(j=i,l=mxExp;j>i-mxExp-1;j--,l--)
                {
                    pTemp->aVecEle[j] = EliminationUnkowns(pField,pTemp->aVecEle[j],
                                   p2->aVecEle[l],pT[0],0);
//                    FieldEle *pEleOut = pTemp->aVecEle[j];
//                    loga("i j %d %d val %.2f",i,j,pEleOut->val);
                }
//                if( p1->eType>0 )
//                {
//                    loga("Tt0");
//                    PrintVal(pField,(VectorEle *)&pT[0],1);
//                    loga("p2");
//                    PrintVal(pField,(VectorEle *)p2->aVecEle,p2->nEle);
//                    loga("TEMP");
//                    PrintVal(pField,(VectorEle **)pTemp->aVecEle,pTemp->nEle);
//                }
                if( bQuotient )
                {
                    pVector->aVecEle[i-mxExp] = pT[0];
                }
                else
                {
                    FreeGroupEle(pPlus,pT[0]);
                }

            }
            else
            {
                pT[0] = VectorDivField(pField,pTemp->aVecEle[i],
                        p2->aVecEle[mxExp]);
                for(j=i,l=p2->nEle-1;j>i-mxExp-1;j--,l--)
                {
                    pTemp->aVecEle[j] = EliminationUnkowns(pField,pTemp->aVecEle[j],
                                   p2->aVecEle[l],pT[0],1);
                }
                if( bQuotient )
                {
                    pVector->aVecEle[i-mxExp] = pT[0];
                }
                else
                {
                    FreeVector(pT[0]);
                }
            }
        }
        if( !bQuotient )
        {
            for(i=0; i<mxExp; i++)
            {
                pVector->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,pTemp->aVecEle[i]);
                //FieldEle *pEleOut = pTemp->aVecEle[i];
                //loga("i %d out %.2f",i,pEleOut->val);
            }
        }
        FreeVector(pTemp);
    }
    else
    {
        pVector = NewVector1(p1,p1->nEle);
        for(i=0; i<p1->nEle; i++)
        {
            if( bQuotient )
            {
                pVector->aVecEle[i] =pPlus->xOperat(pPlus->pBaseEle,pPlus->pBaseEle);
            }
            else
            {
                pVector->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,p1->aVecEle[i]);
            }
        }
    }

    return pVector;
}

void VectorSpaceTest(FieldSys *pField)
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
        paVector[i]->eType = BASE_ELE_TYPE;
        for(j=0;j<ROW;j++)
        {
            k = FakeRand(i+j+5);
            SetGenPara(pField->pGroup2,k);
           // if(j==3)k=0;
            paVector[i]->aVecEle[j] = pField->pGroup1->xGen(pField->pGroup1,k);
            pEle = paVector[i]->aVecEle[j];
            char asym[2] = {'+','-'};
            //logc("%c%d/%d ",asym[pEle->eSymb],pEle->nmrtr,pEle->dnmtr);
            logc("%.2f  ",pEle->val);
        }
        logc("\n");
    }
    VectorEle *pTest;
    FieldEle **paEle;
    pTest = VectorMult(pField,paVector[0],paVector[1]);
    paEle = (FieldEle **)pTest->aVecEle;
    for(i=0; i<pTest->nEle; i++)
    {
        logc("%.2f  ",paEle[i]->val);
    }
    logc("\n");
    pTest->aVecEle[2] = pField->pGroup1->xOperat(pTest->aVecEle[2],pTest->aVecEle[1]);
    pTest = VectorMod(pField,pTest,paVector[1],0);
    paEle = (FieldEle **)pTest->aVecEle;
    for(i=0; i<pTest->nEle; i++)
    {
        logc("%.2f  ",paEle[i]->val);
    }
    logc("\n");

}


int isVecotrEqual(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2)
{
    int rc = 0;
    int nEle = p1->nEle;
    int i;
    FieldEle* pT[2];

    for(i=0; i<nEle; i++)
    {
        pT[0] = p1->aVecEle[i];
        pT[1] = p2->aVecEle[i];
        rc = pField->pGroup1->xIsEqual(pT[0],pT[1]);
        if( !rc )   break;
    }

    return rc;
}

VectorEle *FieldMultVector(
        FieldSys *pField,
        void *pEle,
        VectorEle *p1)
{
    int nEle = p1->nEle;
    VectorEle *pVector;
    int i;

    pVector = NewVector1(p1,p1->nEle);
    for(i=0;i<nEle;i++)
    {
        pVector->aVecEle[i] = pField->pGroup2->xOperat(pEle,
                p1->aVecEle[i]);
    }

    return pVector;
}

void FreeVector(VectorEle *pVec)
{
    int nEle = pVec->nEle;
    int i;

    if( pVec->eType!=BASE_ELE_TYPE )
    {
        for(i=0; i<nEle; i++)
        {
            FreeVector(pVec->aVecEle[i]);
        }
    }
    else
    {
        for(i=0; i<nEle; i++)
        {
            free(pVec->aVecEle[i]);
        }
    }
    free(pVec);
}

//a(A+B) = aA + aB
int VectorDist1(FieldSys *pField,int nEle)
{
    int rc = 0;
    int i,j,l;
    u32 k;
    FieldEle* pFieldEle;
    OperateSys *pMult = pField->pGroup2;
    VectorEle *pVector[2];
    VectorEle *pV[5];

    for(i=0; i<10; i++)
    {

        k = FakeRand(i+j*10);
        SetGenPara(pMult,k);
        pFieldEle = pMult->xGen(pMult,k);

        for(j=0; j<2; j++)
        {
            pVector[j] = (VectorEle *)malloc(sizeof(VectorEle));
            memset(pVector[j],0,sizeof(VectorEle));
            pVector[j]->nEle = nEle;
            pVector[j]->aVecEle = malloc(nEle*sizeof(void *));
            for(l=0;l<nEle; l++)
            {
                k = FakeRand(i+j+l);
                SetGenPara(pMult,k);
                pVector[j]->aVecEle[l] = pMult->xGen(pMult,k);
            }
        }

        pV[0] = VectorPlus(pField,pVector[0],pVector[1]);
        pV[1] = FieldMultVector(pField,pFieldEle,pV[0]);
        pV[2] = FieldMultVector(pField,pFieldEle,pVector[0]);
        pV[3] = FieldMultVector(pField,pFieldEle,pVector[1]);
        pV[4] = VectorPlus(pField,pV[2],pV[3]);
        rc = isVecotrEqual(pField,pV[1],pV[4]);
        free(pFieldEle);
        FreeVector(pVector[0]);
        FreeVector(pVector[1]);
        for(j=0; j<5; j++)
        {
            FreeVector(pV[j]);
        }

        assert( rc );
    }
    loga("vector Distributive1 ok %d",rc);
    return rc;

}

//(a + b)A = aA + bA
int VectorDist2(FieldSys *pField,int nEle)
{
    int rc = 0;
    int i,j,l;
    u32 k;
    FieldEle* pT[3];
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pPlus = pField->pGroup1;
    VectorEle *pV[5];

    for(i=0; i<10; i++)
    {
        for(j=0; j<2; j++)
        {
            k = FakeRand(i+j*10);
            SetGenPara(pMult,k);
            pT[j] = pMult->xGen(pMult,k);
        }


        pV[0] = (VectorEle *)malloc(sizeof(VectorEle));
        memset(pV[0],0,sizeof(VectorEle));
        pV[0]->nEle = nEle;
        pV[0]->aVecEle = malloc(nEle*sizeof(void *));
        for(l=0;l<nEle; l++)
        {
            k = FakeRand(i+l);
            SetGenPara(pMult,k);
            pV[0]->aVecEle[l] = pMult->xGen(pMult,k);
        }

        pT[2] = pPlus->xOperat(pT[0],pT[1]);
        pV[1] = FieldMultVector(pField,pT[2],pV[0]);

        pV[2] = FieldMultVector(pField,pT[0],pV[0]);
        pV[3] = FieldMultVector(pField,pT[1],pV[0]);
        pV[4] = VectorPlus(pField,pV[2],pV[3]);
        rc = isVecotrEqual(pField,pV[1],pV[4]);

        for(j=0; j<3; j++)
        {
            free(pT[j]);
        }
        for(j=0; j<5; j++)
        {
            FreeVector(pV[j]);
        }

        assert( rc );
    }
    loga("vector Distributive2 ok %d",rc);
    return rc;

}

//a(bA) = (ab)A
int VectorAsso(FieldSys *pField,int nEle)
{
    int rc = 0;
    int i,j,l;
    u32 k;
    FieldEle* pT[3];
    OperateSys *pMult = pField->pGroup2;
    VectorEle *pV[4];

    for(i=0; i<10; i++)
    {
        for(j=0; j<2; j++)
        {
            k = FakeRand(i+j*10);
            SetGenPara(pMult,k);
            pT[j] = pMult->xGen(pMult,k);
        }


        pV[0] = (VectorEle *)malloc(sizeof(VectorEle));
        memset(pV[0],0,sizeof(VectorEle));
        pV[0]->nEle = nEle;
        pV[0]->aVecEle = malloc(nEle*sizeof(void *));
        for(l=0;l<nEle; l++)
        {
            k = FakeRand(i+l);
            SetGenPara(pMult,k);
            pV[0]->aVecEle[l] = pMult->xGen(pMult,k);
        }

        pV[1] = FieldMultVector(pField,pT[1],pV[0]);
        pV[2] = FieldMultVector(pField,pT[0],pV[1]);

        pT[2] = pMult->xOperat(pT[0],pT[1]);
        pV[3] = FieldMultVector(pField,pT[2],pV[0]);
        rc = isVecotrEqual(pField,pV[2],pV[3]);

        for(j=0; j<3; j++)
        {
            free(pT[j]);
        }
        for(j=0; j<4; j++)
        {
            FreeVector(pV[j]);
        }

        assert( rc );
    }
    loga("vector Association ok %d",rc);
    return rc;
}

int VectorIdentity(FieldSys *pField,int nEle)
{
    int rc = 0;
    int i,j,l;
    u32 k;
    OperateSys *pMult = pField->pGroup2;
    VectorEle *pV[2];

    for(i=0; i<10; i++)
    {

        pV[0] = (VectorEle *)malloc(sizeof(VectorEle));
        memset(pV[0],0,sizeof(VectorEle));
        pV[0]->nEle = nEle;
        pV[0]->aVecEle = malloc(nEle*sizeof(void *));
        for(l=0;l<nEle; l++)
        {
            k = FakeRand(i+l);
            SetGenPara(pMult,k);
            pV[0]->aVecEle[l] = pMult->xGen(pMult,k);
        }

        pV[1] = FieldMultVector(pField,pMult->pBaseEle,pV[0]);
        rc = isVecotrEqual(pField,pV[0],pV[1]);

        for(j=0; j<2; j++)
        {
            FreeVector(pV[j]);
        }

        assert( rc );
    }
    loga("vector Identity ok %d",rc);
    return rc;
}


void *SolveOnceEqu(
        FieldSys *pField,
        VectorEle **paVector,//输入的向量组
        VectorEle *pRight,//等式右边向量
        VectorEle *pSolve,
        int iCol,
        int iRow,
        int n
        )
{
    FieldEle *pT[10] = {0};
    FieldEle *pEle;
    FieldEle *pSum;
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    int i;

    for(i=1; i<n; i++)
    {
        if( 1==i )
        {
            pEle = paVector[i]->aVecEle[iRow];
            pT[0] = pMult->xOperat(pEle,pSolve->aVecEle[iCol+i]);
        }
        else
        {
            pSum = pT[0];
            pEle = paVector[i]->aVecEle[iRow];
            pT[1] = pMult->xOperat(pEle,pSolve->aVecEle[iCol+i]);
            pT[0] = pPlus->xOperat(pSum,pT[1]);
            FreeGroupEle(pMult,pT[1]);
            FreeGroupEle(pMult,pSum);
        }
    }
    pT[2] = pPlus->xInvEle(pT[0]);
    pT[3] = pPlus->xOperat(pT[2],pRight->aVecEle[iRow]);
    pT[4] = pMult->xInvEle(paVector[0]->aVecEle[iRow]);
    pT[5] = pMult->xOperat(pT[4],pT[3]);
    FreeGroupEle(pMult,pT[0]);
    FreeGroupEle(pMult,pT[2]);
    FreeGroupEle(pMult,pT[3]);
    FreeGroupEle(pMult,pT[4]);

    return pT[5];
}

#define SOLVE_DEBUG 0
int SolveLinearEqu(
        FieldSys *pField,
        VectorEle **paVector,//输入的向量组
        VectorEle *pRight,//等式右边向量
        VectorEle *pSolve,
        int iCol,//起始列
        int n,//n个变量
        int iRow,//输入的方起始行
        int nRow
        )
{
    int rc = 0;
    int i,j;
    OperateSys *pPlus = pField->pGroup1;
    OperateSys *pMult = pField->pGroup2;
    FieldEle **paEle;
    FieldEle *pTemp;
    FieldEle *pTempEle;
    FieldEle *pT[10] = {0};

    assert(n>0);
    if( n==1 && nRow>0 )
    {
#if SOLVE_DEBUG
        if(pField->pGroup1->typeEle>0)
        {
            loga("n col %d",n);
            for(i=0;i<n;i++)
            {
                PrintVal(pField,paVector[i]->aVecEle,paVector[i]->nEle);
            }
            loga("rigth %d",n);
            PrintVal(pField,pRight->aVecEle,pRight->nEle);
        }
        else
        {
            loga("n col %d",n);
            PrintVal(pField,paVector,n);
            loga("rigth %d",n);
            PrintVal(pField,&pRight,1);
        }
#endif
        //不断消元递归后只剩一个变量，可能还有多个方程
        //这几个方程的解相同才可能有解
        paEle= (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        for(i=0; i<nRow; i++)
        {
            if( !pPlus->xIsEqual(paEle[i],pPlus->pBaseEle) )
            {
                if( pT[1]==NULL )
                {
                    pT[0] = pMult->xInvEle(paEle[i]);
                    pT[1] = pMult->xOperat(pT[0],pRight->aVecEle[iRow+i]);
#if SOLVE_DEBUG
                    if(pField->pGroup1->typeEle>0)
                    {
                        loga("solve ");
                        PrintVal(pField,&paEle[i],1);
                        PrintVal(pField,&pRight->aVecEle[iRow+i],1);
                        PrintVal(pField,&pT[1],1);
                        loga("inv");
                        PrintVal(pField,&pT[0],1);
                        VectorEle *test;
                        FieldSys * sub = (( VectorEle *)pT[1])->pSubField;
                        VectorEle *poly = (( VectorEle *)pT[1])->pPoly;
                        PrintVal(pField,&poly,1);
                        test = VectorMult(sub,pT[1],paEle[i]);
                        PrintVal(pField,&test,1);
                        test = pMult->xOperat(pT[0],paEle[i]);
                        PrintVal(pField,&test,1);
                    }
#endif
                    FreeGroupEle(pMult,pT[0]);
                }
                else
                {
                    pT[0] = pMult->xInvEle(paEle[i]);
                    pT[2] = pMult->xOperat(pT[0],pRight->aVecEle[iRow+i]);
                    if( !pPlus->xIsEqual(pT[1],pT[2]) )
                    {
                        FreeGroupEle(pMult,pT[0]);
                        FreeGroupEle(pMult,pT[1]);
                        FreeGroupEle(pMult,pT[2]);
                        return 0;
                    }
                    FreeGroupEle(pMult,pT[0]);
                    FreeGroupEle(pMult,pT[2]);
                }
            }
            else if( !pPlus->xIsEqual(pRight->aVecEle[iRow+i],pPlus->pBaseEle) )
            {
                //左边为0，右边不为0，无解
                FreeGroupEle(pMult,pT[1]);
                return 0;
            }
            else
            {
                //左右都为0取单位元
                pT[1] = pPlus->xOperat(pPlus->pBaseEle,pMult->pBaseEle);
            }
        }
        //循环没返回说明这几个方程有公共解pT[1]
        pSolve->aVecEle[iCol] = pT[1];

        return 1;
    }
    else if( nRow==0 )
    {
        for(i=0; i<n; i++)
        {
            pSolve->aVecEle[iCol+i] = pPlus->xOperat(
                    pPlus->pBaseEle,pMult->pBaseEle);
        }
        //只剩一行，但是未知变量大于1，那么必定有解
        return 1;
    }
    else
    {
#if SOLVE_DEBUG
        if(pField->pGroup1->typeEle>0)
        {
            loga("n col %d",n);
            for(i=0;i<n;i++)
            {
                PrintVal(pField,paVector[i]->aVecEle,paVector[i]->nEle);
            }
            loga("rigth %d",n);
            PrintVal(pField,pRight->aVecEle,pRight->nEle);
        }
        else
        {
            loga("n col %d",n);
            PrintVal(pField,paVector,n);
            loga("rigth %d",n);
            PrintVal(pField,&pRight,1);
        }
#endif
        paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        for(i=0; i<nRow; i++)
        {
            //找到第1列的非0元素，换到第一行
            if( !pPlus->xIsEqual(paEle[i],pPlus->pBaseEle) )
            {
                for(j=0;j<=n;j++)
                {
                    if( j==n )
                    {
                        paEle = (FieldEle **)&pRight->aVecEle[iRow];
                    }
                    else
                    {
                        paEle = (FieldEle **)&((paVector[j])->aVecEle[iRow]);
                    }
                    pTempEle = paEle[i];
                    paEle[i] = paEle[0];
                    paEle[0] = pTempEle;
                }
                break;
            }
        }
        paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        //如果全为0，则取单位元，不影响方程解
        if( pPlus->xIsEqual(paEle[0],pPlus->pBaseEle) )
        {
            rc = SolveLinearEqu(pField,&paVector[1],pRight,pSolve,iCol+1,n-1,iRow,nRow);
            if( rc )
            {
                pSolve->aVecEle[iCol] = pPlus->xOperat(pPlus->pBaseEle,pMult->pBaseEle);
            }
        }
        else
        {
            for(i=1; i<nRow; i++)
            {
                paEle = (FieldEle **)&((paVector[0])->aVecEle[iRow]);
                pTemp = FiledDiv(pField,paEle[i],paEle[0]);
#if SOLVE_DEBUG
                //打印消元时的比值
                if(pField->pGroup1->typeEle>0)
                {
                    loga("i %d",i);
                    PrintVal(pField,&pTemp,1);
                }
                else
                {
                    if( iRow==2 && i==1 && pTemp->val>4.1 && pTemp->val<4.3)
                    {
                        assert(0);
                    }
                    loga("row %d i %d val %.2f",iRow,i,pTemp->val);
                }
#endif
                //loga("row %d i %d val %.2f",iRow,i,pTemp->val);
                //pTemp是后面方程的元素与第1个方程元素的比值
                for(j=0;j<n;j++)
                {
                    paEle = (FieldEle **)&((paVector[j])->aVecEle[iRow]);
                    //根据这个比值消元
                    paEle[i] = EliminationUnkowns(pField,paEle[i],paEle[0],pTemp,0);
                }
                paEle = (FieldEle **)&(pRight->aVecEle[iRow]);
                paEle[i] = EliminationUnkowns(pField,paEle[i],paEle[0],pTemp,0);
                FreeGroupEle(pMult,pTemp);
            }
            //消去第一个变量后，递归下一列和下一行，重新执行以上步骤
            rc = SolveLinearEqu(pField,&paVector[1],pRight,pSolve,iCol+1,n-1,iRow+1,nRow-1);
            if( rc )
            {
                pSolve->aVecEle[iCol] = SolveOnceEqu(pField,paVector,pRight,pSolve,iCol,iRow,n);
            }
        }
    }

    return rc;
}

int isLinearDepedent(
        FieldSys *pField,
        VectorEle **paVector,//输入的向量组
        int n,//n个变量
        int iRow,//输入的方程起始编号
        int nRow)//输入的方程个数
{
    int rc = 0;
    int i,j;
    OperateSys *pPlus = pField->pGroup1;
    FieldEle **paEle;
    FieldEle *pTemp;
    FieldEle *pTempEle;

    assert(n>0);
    assert(nRow>0);
    if( n==1 )
    {
        //不断消元递归后只剩一个变量，可能还有多个方程
        paEle= (FieldEle **)&((paVector[0])->aVecEle[iRow]);
        for(i=0; i<nRow; i++)
        {
            //线性变换后，最后一列有非零元素，所以不可能存在非零解，线性无关
            if( !pPlus->xIsEqual(paEle[i],pPlus->pBaseEle) )
            {
                return 0;
            }
        }
        //如果最后一列系数全是0，那么有非零解，线性相关
        return 1;
    }
    else if( nRow==1 )
    {
        //只剩一行，但是未知变量大于1，那么必定有非零解
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
        //如果第一列的元素全为0，那么忽略这一列，递归下一列
        //todo 此时是否可以直接认定为线性相关
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
                //pTemp是后面方程的元素与第1个方程元素的比值
                for(j=0;j<n;j++)
                {
                    paEle = (FieldEle **)&((paVector[j])->aVecEle[iRow]);
                    //根据这个比值消元
                    paEle[i] = EliminationUnkowns(pField,paEle[i],paEle[0],pTemp,0);
                }
                free(pTemp);
            }
            //消去第一个变量后，递归下一列和下一行，重新执行以上步骤
            rc = isLinearDepedent(pField,&paVector[1],n-1,iRow+1,nRow-1);
        }
    }

    return rc;
}

void PrintVec(FieldSys *pField,VectorEle *p)
{
    if( p->eType<2 )
    {
        PrintVal(pField,(VectorEle **)p->aVecEle,p->nEle);
    }
    else
    {
        VectorEle **paVec = (VectorEle **)p->aVecEle;
        for(int i=0; i<p->nEle; i++)
        {
            PrintVal(pField,(VectorEle **)paVec[i]->aVecEle,paVec[i]->nEle);
        }
    }
}
void PrintVal(
        FieldSys *pField,
        VectorEle **paVector,
        int nCol)
{
    int i,j;

    FieldEle *pEle;

    if( nCol==1 )
    {
        for(j=0;j<paVector[0]->nEle;j++)
        {
            pEle = paVector[0]->aVecEle[j];
            logc("%-6.2f  ",pEle->val);
        }
        logc("\n");
    }
    else
    {
        for(j=0;j<paVector[0]->nEle;j++)
        {
            for(i=0; i<nCol; i++)
            {
                pEle = paVector[i]->aVecEle[j];
                logc("%-6.2f  ",pEle->val);
            }
            logc("\n");
        }
    }
    logc("\n");
}

void isVectorSpace(FieldSys *pField,int nEle)
{
    VectorDist1(pField,nEle);
    VectorDist2(pField,nEle);
    VectorAsso(pField,nEle);
    VectorIdentity(pField,nEle);
}

void VectorTest(FieldSys *pField)
{
    int rc;
    VectorSpaceTest(pField);
    rc = isLinearDepedent(pField,pField->paVector,COL,0,ROW);
    loga("isLinear %d",rc);
    PrintVal(pField,pField->paVector,COL);
    loga("--");
    isVectorSpace(pField,4);
}
