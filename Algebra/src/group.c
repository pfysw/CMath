/*
 * group.c
 *
 *  Created on: Apr 11, 2019
 *      Author: Administrator
 */
#include "group.h"
#include "algebra.h"
#include "field.h"
#include "linear.h"

typedef struct FivePerm
{
    u8 aNum[5];
}FivePerm;

void SwapNum(u8 *a, u8 *b)
{
    int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void FivePermTrav(FivePerm *pPerm, int left, int right, int num)
{
    static int cnt = 0;
    static int RecCnt = 0;
    int i;

    if( RecCnt==0 )
    {
        cnt = 0;
    }
    RecCnt++;
    if( cnt>num )
    {
        RecCnt--;
        return;
    }
    if( left==right )
    {
        cnt++;
//        loga("cnt %d ",cnt);
//        memout(&pPerm->aNum[0],5);
    }
    else
    {
        for(i=left; i<=right; i++)
        {
            SwapNum(&pPerm->aNum[i],&pPerm->aNum[left]);
            FivePermTrav(pPerm,left+1,right,num);
            if( cnt>num )
            {
                break;
            }
            SwapNum(&pPerm->aNum[i],&pPerm->aNum[left]);

        }
    }
    RecCnt--;
}

FivePerm *FivePermGen(OperateSys *pOpSys,u32 num)
{
    FivePerm *p;
    u8 aInit[5] = {1,2,3,4,5};
    num = num%120;
    p = malloc(sizeof(FivePerm));
    memcpy(p->aNum,aInit,sizeof(aInit));
    FivePermTrav(p,0,4,num);
    return p;
}

FivePerm *FivePermRec(FivePerm *pPrev)
{
    FivePerm *p;
    return p;
}

int FivePermEqual(FivePerm *p1, FivePerm *p2)
{
    int rc = 1;
    int i;

    for(i=0; i<5; i++)
    {
        if( p1->aNum[i]!=p2->aNum[i] )
        {
            rc = 0;
            break;
        }
    }

    return rc;
}

FivePerm *FivePermInv(FivePerm *p1)
{
    FivePerm *p;
    int i,j;
    p = malloc(sizeof(FivePerm));
    memset(p,0,sizeof(FivePerm));

    //无论是求左逆元还是右逆元在代码上的形式是相同的
    for(i=0; i<5; i++)
    {
        j = p1->aNum[i];
        p->aNum[j-1] = i+1;
    }

    return p;
}

FivePerm *FivePermOp(FivePerm *p1, FivePerm *p2)
{
    FivePerm *p;
    int i,j;
    p = malloc(sizeof(FivePerm));
    memset(p,0,sizeof(FivePerm));

    for(i=0; i<5; i++)
    {
        j = p2->aNum[i]-1;
        p->aNum[i] = p1->aNum[j];
    }

    return p;
}

//根据p1的轮换来对p2进行置换
FivePerm *FivePermOp1(FivePerm *p1, FivePerm *p2)
{
    FivePerm *p;
    int i,j;
    p = malloc(sizeof(FivePerm));
    memset(p,0,sizeof(FivePerm));
    for(i=0; i<5; i++)
    {
        j = p1->aNum[i]-1;
        p->aNum[i] = p2->aNum[j];
    }
    return p;
}

OperateSys *PermutationObj(void)
{
    static FivePerm baseItem =
    {
        .aNum[0] = 1,
        .aNum[1] = 2,
        .aNum[2] = 3,
        .aNum[3] = 4,
        .aNum[4] = 5,
    };

    //函数类型不匹配，加void去警告
    static OperateSys perm;
    memset(&perm,0,sizeof(perm));

    perm.pBaseEle = &baseItem;
    perm.nPara = 1;
    perm.xIsEqual = (void*)FivePermEqual;
    perm.xGen = (void*)FivePermGen;
    perm.xInvEle = (void*)FivePermInv;
    perm.xOperat =  (void*)FivePermOp;

    return &perm;
}

void SetGenPara(OperateSys* pOpSys, u32 iNum)
{
    int i;
    if( pOpSys->nPara>1 )
    {
        for(i=0; i<pOpSys->nPara-1; i++)
        {
            pOpSys->aGenPara[i] = FakeRand(iNum+i);
        }
    }
}

void *GenRatuonEle(OperateSys *pOpSys, int k)
{
    k = FakeRand(k);
    SetGenPara(pOpSys,k);
    return pOpSys->xGen(pOpSys,k);
}

void FreeGroupEle(OperateSys *pOpSys,void *p)
{
    if( p==NULL )
    {
        return;
    }
    if( pOpSys->typeEle==FIELD_ELE )
    {
        free(p);
    }
    else if( pOpSys->typeEle==VEC_ELE )
    {
        FreeVector((VectorEle *)p);
    }

}
int AssociativeLaw(OperateSys *pOpSys)
{
    int rc = 0;
    int i,j,k;
    //FieldEle* pT[7];
    VectorEle* pT[7];

    for(i=0; i<10; i++)
    {

        for(j=0; j<3; j++)
        {
            k = FakeRand(i+j*10);
            SetGenPara(pOpSys,k);

            pT[j] = pOpSys->xGen(pOpSys,k);
           // loga("i %d j %d k %d",i,j,k%120);
//            memout(&((FivePerm*)pT[j])->aNum[0],5);
        }
        //loga("----");
        pT[3] = pOpSys->xOperat(pT[1],pT[2]);
        pT[4] = pOpSys->xOperat(pT[0],pT[1]);
        pT[5] = pOpSys->xOperat(pT[0],pT[3]);
        pT[6] = pOpSys->xOperat(pT[4],pT[2]);

        rc = pOpSys->xIsEqual(pT[5],pT[6]);

        for(j=0; j<7; j++)
        {
            FreeGroupEle(pOpSys,pT[j]);
        }

        assert( rc );
    }

    loga("associative ok");

    return rc;
}

int HasInvEle(OperateSys *pOpSys)
{
    int rc = 0;
    int i,k;
//    void* pEle;
//    void* pGen;
//    void* pInv;

    VectorEle* pEle;
    VectorEle* pGen;
    VectorEle* pInv;

    for(i=0; i<10; i++)
    {
        //debug
//        static int jj=0;
//        jj++;
//        if(jj==41)
//        {
//            loga("ss");
//        }
//        loga("jj %d",jj);


        k = FakeRand(i+2);
        SetGenPara(pOpSys,k);
        pGen = pOpSys->xGen(pOpSys,k);

//        if( pOpSys->isMult )
//        {
//            FieldEle **paEle;
//            paEle = (FieldEle **)pGen->aVecEle;
//            for(int ii=0; ii<pGen->nEle; ii++)
//            {
//                logc("%.2f  ",paEle[ii]->val);
//            }
//            logc("\n");
//        }

        pInv = pOpSys->xInvEle(pGen);
       // pEle = pOpSys->xOperat(pGen,pInv);
        pEle = pOpSys->xOperat(pInv,pGen);
        rc = pOpSys->xIsEqual(pEle,pOpSys->pBaseEle);

//        if(!rc)
//        {
//            sleep(1);
//        }

        FreeGroupEle(pOpSys,pGen);
        FreeGroupEle(pOpSys,pEle);
        FreeGroupEle(pOpSys,pInv);
        assert( rc );
    }

    loga("inv ok %d",rc);
    return rc;
}

int HasIdentityEle(OperateSys *pOpSys)
{
    int rc = 0;
    int i,k;
    void* pEle;
    void* pGen;
    for(i=0; i<10; i++)
    {
        k = FakeRand(i);
        SetGenPara(pOpSys,k);
        pGen = pOpSys->xGen(pOpSys,k);
        pEle = pOpSys->xOperat(pOpSys->pBaseEle,pGen);
        rc = pOpSys->xIsEqual(pGen,pEle);

        FreeGroupEle(pOpSys,pGen);
        FreeGroupEle(pOpSys,pEle);
        assert( rc );
    }

    loga("identity ok %d",rc);
    return rc;
}

void PermPrintTest(OperateSys *pOpSys)
{
    FivePerm* pTest;
    FivePerm* pT[5];

    loga("sd %d",((FivePerm*)pOpSys->pBaseEle)->aNum[3]);
    pT[0] = pOpSys->xGen(pOpSys,1);
//    sleep(1);
//    assert(0);
    pT[1] = pOpSys->xGen(pOpSys,92);
    pT[2] = pOpSys->xGen(pOpSys,72);
    for(int i=0; i<3; i++)
    {
        memout(&pT[i]->aNum[0],5);
    }
    loga("----");
    FivePerm p1 = {
        {1,3,4,5,2}
    };
    FivePerm p2 = {
        {2,3,5,4,1}
    };
    loga("is p1==p1 : %d",pOpSys->xIsEqual(&p1,&p2));
    pTest = pOpSys->xOperat(&p1,&p2);
    pT[3] = pOpSys->xOperat(pT[2],pTest);
    loga("pT3");
    memout(&pT[3]->aNum[0],5);
    FreeGroupEle(pOpSys,pTest);
    pTest = pOpSys->xOperat(pT[2],&p1);
    pT[4] = pOpSys->xOperat(pTest,&p2);
    loga("pT4");
    memout(&pT[4]->aNum[0],5);
    FreeGroupEle(pOpSys,pTest);
    loga("is pT3==pT4 : %d",pOpSys->xIsEqual(pT[3],pT[4]));
    pTest = pOpSys->xInvEle(&p1);
    loga("inv p1");
    memout(&pTest->aNum[0],5);

    pT[3] = pOpSys->xOperat(pTest,&p1);
    loga("pT31");
    memout(&pT[3]->aNum[0],5);
    loga("");
   // pTest =FivePermOp1(&p1,&p2);
   // memout(&pTest->aNum[0],5);
    IsGroup(pOpSys);
}

void SetOperaSys(OperateSys **ppOpSys)
{
    *ppOpSys = PermutationObj();
}


