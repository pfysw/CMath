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

    pVector = (VectorEle *)malloc(sizeof(VectorEle));
    memset(pVector,0,sizeof(VectorEle));
    pVector->nEle = nEle;
    pVector->aVecEle = malloc(nEle*sizeof(void *));
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
    FieldEle *pT[2];
    int i,j,k;
    int size;

    pVector = (VectorEle *)malloc(sizeof(VectorEle));
    memset(pVector,0,sizeof(VectorEle));
    pVector->nEle = p1->nEle+p2->nEle-1;
    size = pVector->nEle*sizeof(void *);
    pVector->aVecEle = malloc(size);
    memset(pVector->aVecEle,0,size);
    for(i=0;i<p1->nEle;i++)
    {
        for(j=0;j<p2->nEle;j++)
        {
            k = i+j;
            pT[0] = pMult->xOperat(p1->aVecEle[i],p2->aVecEle[j]);
            if( pVector->aVecEle[k]==NULL )
            {
                pVector->aVecEle[k] = pT[0];
            }
            else
            {
                pT[1] = pPlus->xOperat(pT[0],pVector->aVecEle[k]);
                free(pVector->aVecEle[k]);
                free(pT[0]);
                pVector->aVecEle[k] = pT[1];
            }

        }
    }

    return pVector;
}

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

VectorEle *VectorMod(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2)
{
    VectorEle *pVector;
    VectorEle *pTemp;
    OperateSys *pPlus = pField->pGroup1;
    FieldEle *pT[2];
    int i,j,l;

    pTemp = NewVector(p1->nEle);
    for(i=0; i<p1->nEle; i++)
    {
        pTemp->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,p1->aVecEle[i]);
    }
    if( p1->nEle>=p2->nEle )
    {
        pVector = NewVector(p2->nEle-1);
        for(i=p1->nEle-1;i>=p2->nEle-1;i--)
        {
            pT[0] = FiledDiv(pField,pTemp->aVecEle[i],
                    p2->aVecEle[p2->nEle-1]);
            for(j=i,l=p2->nEle-1;j>i-p2->nEle;j--,l--)
            {
                pTemp->aVecEle[j] = EliminationUnkowns(pField,pTemp->aVecEle[j],
                               p2->aVecEle[l],pT[0]);
            }
            free(pT[0]);
        }
        for(i=0; i<p2->nEle-1; i++)
        {
            pVector->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,pTemp->aVecEle[i]);
        }
        FreeVector(pTemp);
    }
    else
    {
        pVector = NewVector(p1->nEle);
        for(i=0; i<p1->nEle; i++)
        {
            pVector->aVecEle[i] = pPlus->xOperat(pPlus->pBaseEle,p1->aVecEle[i]);
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
        for(j=0;j<ROW;j++)
        {
            k = FakeRand(i+j+5);
            SetGenPara(pField->pGroup2,k);
           // if(j==3)k=0;
            paVector[i]->aVecEle[j] = pField->pGroup2->xGen(pField->pGroup2,k);
            pEle = paVector[i]->aVecEle[j];
            char asym[2] = {'+','-'};
            //logc("%c%d/%d ",asym[pEle->eSymb],pEle->nmrtr,pEle->dnmtr);
            logc("%.2f  ",pEle->val);
        }
        logc("\n");
    }
    VectorEle *pTest;
    pTest = VectorMult(pField,paVector[0],paVector[1]);
    for(i=0; i<pTest->nEle; i++)
    {
        logc("%.2f  ",pTest->aVecEle[i]->val);
    }
    logc("\n");
    pTest->aVecEle[2] = pField->pGroup1->xOperat(pTest->aVecEle[2],pTest->aVecEle[1]);
    pTest = VectorMod(pField,pTest,paVector[1]);
    for(i=0; i<pTest->nEle; i++)
    {
        logc("%.2f  ",pTest->aVecEle[i]->val);
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
        VectorEle *pVector1)
{
    int nEle = pVector1->nEle;
    VectorEle *pVector;
    int i;

    pVector = (VectorEle *)malloc(sizeof(VectorEle));
    memset(pVector,0,sizeof(VectorEle));
    pVector->nEle = nEle;
    pVector->aVecEle = malloc(nEle*sizeof(void *));
    for(i=0;i<nEle;i++)
    {
        pVector->aVecEle[i] = pField->pGroup2->xOperat(pEle,
                pVector1->aVecEle[i]);
    }

    return pVector;
}

void FreeVector(VectorEle *pVec)
{
    int nEle = pVec->nEle;
    int i;

    for(i=0; i<nEle; i++)
    {
        free(pVec->aVecEle[i]);
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
                    //根据这个比值校园
                    paEle[i] = EliminationUnkowns(pField,paEle[i],paEle[0],pTemp);
                }
                free(pTemp);
            }
            //消去第一个变量后，递归下一列和下一行，重新执行以上步骤
            rc = isLinearDepedent(pField,&paVector[1],n-1,iRow+1,nRow-1);
        }
    }

    return rc;
}

void PrintVal(
        FieldSys *pField,
        VectorEle **paVector)
{
    int i,j;

    FieldEle *pEle;
    for(i=0; i<COL; i++)
    {
        for(j=0;j<ROW;j++)
        {
            pEle = paVector[i]->aVecEle[j];
            logc("%.2f  ",pEle->val);
        }
        logc("\n");
    }
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
    PrintVal(pField,pField->paVector);
    loga("--");
    isVectorSpace(pField,4);
}
