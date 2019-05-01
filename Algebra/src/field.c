/*
 * field.c
 *
 *  Created on: Apr 16, 2019
 *      Author: Administrator
 */
#include"algebra.h"
#include "field.h"
#include "group.h"

typedef enum RATION_TYPE{
    POS_NUM,
    NEG_NUM
}RATION_TYPE;



u32 IntegerGcd(u32 a, u32 b)
{
    if( b==0 )
    {
        return a;
    }
    else
    {
        return IntegerGcd(b,a%b);
    }

}

int RationEqual(FieldEle *p1, FieldEle *p2)
{
    int rc = 0;

#if 0//判断分数是否相等
    if( p1->dnmtr*p2->nmrtr==p2->dnmtr*p1->nmrtr )
    {
        if( p1->eSymb==p2->eSymb )
        {
            rc = 1;
        }
    }
#else
    if( p1->val>p2->val-0.01 &&
            p1->val<p2->val+0.01 )
    {
        rc = 1;
    }
//    else
//    {
//        loga("delta %f",p1->val-p2->val);
//    }

#endif

    return rc;
}

//为了减少处理很多没有价值的细节
//这里把范围限制在100以内
FieldEle *NatureGen(OperateSys *pOpSys,u32 num)
{

    FieldEle *p;
    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));

    p->nmrtr = num%100;
    p->dnmtr = 1;

    p->eSymb = pOpSys->aGenPara[0]%2;

    p->val = p->nmrtr;
    if( p->eSymb==NEG_NUM )
    {
        p->val = -p->val;
    }

    return p;
}

ModEle *ModNumGen(OperateSys *pOpSys,u32 num)
{

    ModEle *p;
    p = malloc(sizeof(ModEle));
    memset(p,0,sizeof(ModEle));

    p->mod = ((ModEle *)pOpSys->pBaseEle)->mod;
    if( pOpSys->isMult )
    {
        p->num = num%(p->mod-1)+1;
    }
    else
    {
        p->num = num%100;//只产生100以内的数
    }



    return p;
}

int ModEqual(ModEle *p1, ModEle *p2)
{
    int rc = 0;
    int mod = p1->mod;

    if( p1->num%mod==p2->num%mod )
    {
        rc = 1;
    }

    return rc;
}

FieldEle *RationGen(OperateSys *pOpSys,u32 num)
{
    FieldEle *p;
    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));
    u32 dnmtr;

    num = num%100;
    dnmtr = pOpSys->aGenPara[0]%100;

    p->nmrtr = num+1;
    p->dnmtr = dnmtr+1;
    p->eSymb = pOpSys->aGenPara[1]&1;
    p->val = (float)p->nmrtr/p->dnmtr;
    if( p->eSymb==NEG_NUM )
    {
        p->val = -p->val;
    }

    return p;
}

FieldEle *RationGen1(u32 n1,u32 n2,u8 symb)
{

    FieldEle *p;
    u32 div;

    assert(n2!=0);
    assert(symb<2);
    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));
    div = IntegerGcd(n1,n2);
    p->dnmtr = n2/div;
    p->nmrtr = n1/div;
    p->eSymb = symb;

    return p;
}

FieldEle *RationGen2(
        OperateSys *pOpSy,
        u32 n1,
        u32 n2,
        u8 symb)
{
    FieldEle *p;

    pOpSy->aGenPara[0] = n2;
    pOpSy->aGenPara[1] = symb;
    p = pOpSy->xGen(pOpSy,n1);

    return p;
}

FieldEle *RationPlusInv(FieldEle *p1)
{
    FieldEle *p;
    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));

    p->dnmtr = p1->dnmtr;
    p->nmrtr = p1->nmrtr;
    if( p1->eSymb==POS_NUM )
    {
        p->eSymb = NEG_NUM;
    }
    else
    {
        p->eSymb = POS_NUM;
    }

    p->val = -p1->val;

    return p;
}

FieldEle *RationMultInv(FieldEle *p1)
{
    FieldEle *p;
    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));

    p->dnmtr = p1->nmrtr;
    p->nmrtr = p1->dnmtr;
    p->eSymb = p1->eSymb;

    p->val = 1/p1->val;

    return p;
}

FieldEle *RationPlusOp(FieldEle *p1,FieldEle *p2)
{
    FieldEle *p;
    int a,b;
    int sum;
    int aSymb[2] = {1,-1};
    int div;

    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));

    p->dnmtr = p1->dnmtr*p2->dnmtr;

    a = aSymb[p1->eSymb]*p2->dnmtr*p1->nmrtr;
    b = aSymb[p2->eSymb]*p1->dnmtr*p2->nmrtr;

    sum = a+b;


    if(sum>=0)
    {
        p->nmrtr = sum;
        p->eSymb = POS_NUM;
    }
    else
    {
        p->nmrtr = -sum;
        p->eSymb = NEG_NUM;
    }
    div = IntegerGcd(p->nmrtr,p->dnmtr);
    p->dnmtr = p->dnmtr/div;
    p->nmrtr = p->nmrtr/div;

    p->val = p1->val+p2->val;
    return p;
}

ModEle *ModPlus(ModEle *p1,ModEle *p2)
{
    ModEle *p;
    u32 mod = p1->mod;
    p = malloc(sizeof(ModEle));
    memset(p,0,sizeof(ModEle));

    p->mod = mod;
    p->num = (p1->num+p2->num)%mod;

    return p;
}

ModEle *ModMult(ModEle *p1,ModEle *p2)
{
    ModEle *p;
    u32 mod = p1->mod;
    p = malloc(sizeof(ModEle));
    memset(p,0,sizeof(ModEle));

    p->mod = mod;
    p->num = (p1->num*p2->num)%mod;

    return p;
}

ModEle *ModPlusInv(ModEle *p1)
{
    ModEle *p;
    u32 mod = p1->mod;
    p = malloc(sizeof(ModEle));
    memset(p,0,sizeof(ModEle));

    p->mod = mod;
    p->num = mod-p1->num%mod;

    return p;
}

ModEle *ModMultInv(ModEle *p1)
{
    ModEle *p;
    int i;
    u32 mod = p1->mod;
    p = malloc(sizeof(ModEle));
    memset(p,0,sizeof(ModEle));

    p->mod = mod;
    p->num = 1;
    for(i=0; i<mod-2; i++)
    {
        p->num = (p->num*p1->num)%mod;
    }

   // loga("mod %d num %d",mod,p->num);
    return p;
}

FieldEle *RationMultOp(FieldEle *p1,FieldEle *p2)
{
    FieldEle *p;
    int div;

    p = malloc(sizeof(FieldEle));
    memset(p,0,sizeof(FieldEle));

    p->nmrtr = p1->nmrtr*p2->nmrtr;
    p->dnmtr = p1->dnmtr*p2->dnmtr;
    if( p1->eSymb==p2->eSymb ||
            p->nmrtr==0 )
    {
        p->eSymb = POS_NUM;
    }
    else
    {
        p->eSymb = NEG_NUM;
    }

    div = IntegerGcd(p->nmrtr,p->dnmtr);
    p->dnmtr = p->dnmtr/div;
    p->nmrtr = p->nmrtr/div;

    p->val = p1->val*p2->val;
    return p;
}


OperateSys *ModPlusObj(void)
{
    static ModEle baseItem =
    {
        0,
        5,
    };

    static OperateSys plus;
    memset(&plus,0,sizeof(plus));

    plus.pBaseEle = &baseItem;
    plus.nPara = 1;
    plus.xIsEqual = (void*)ModEqual;
    plus.xGen = (void*)ModNumGen;
    plus.xInvEle = (void*)ModPlusInv;
    plus.xOperat =  (void*)ModPlus;

    return &plus;
}

OperateSys *ModMultObj(void)
{
    static ModEle baseItem =
    {
        1,
        5,
    };

    static OperateSys mult;
    memset(&mult,0,sizeof(mult));

    mult.pBaseEle = &baseItem;
    mult.nPara = 1;
    mult.isMult = 1;
    mult.xIsEqual = (void*)ModEqual;
    mult.xGen = (void*)ModNumGen;
    mult.xInvEle = (void*)ModMultInv;
    mult.xOperat =  (void*)ModMult;

    return &mult;
}


OperateSys *RationPlusObj(void)
{
    static FieldEle baseItem =
    {
        0,
        1,
        0,
        POS_NUM,
    };

    static OperateSys plus;
    memset(&plus,0,sizeof(plus));

    plus.pBaseEle = &baseItem;
    plus.nPara = 2;
    plus.xIsEqual = (void*)RationEqual;
    plus.xGen = (void*)NatureGen;
    plus.xInvEle = (void*)RationPlusInv;
    plus.xOperat =  (void*)RationPlusOp;

    return &plus;
}

OperateSys *RationMultObj(void)
{
    static FieldEle baseItem =
    {
        1,
        1,
        1,
        POS_NUM,
    };

    static OperateSys mult;
    memset(&mult,0,sizeof(mult));

    mult.pBaseEle = &baseItem;
    mult.nPara = 3;
    mult.xIsEqual = (void*)RationEqual;
    mult.xGen = (void*)RationGen;
    mult.xInvEle = (void*)RationMultInv;
    mult.xOperat =  (void*)RationMultOp;

    return &mult;
}


void SetFieldSys(FieldSys **ppField)
{
    *ppField = (FieldSys *)malloc(sizeof(FieldSys));
    (*ppField)->pGroup1 = RationPlusObj();
    (*ppField)->pGroup2 = RationMultObj();

//    (*ppField)->pGroup1 = ModPlusObj();
//    (*ppField)->pGroup2 = ModMultObj();
}

int DistributiveLaw(FieldSys *pField)
{
    int rc = 0;
    int i,j;
    u32 k;
    //void* pT[8];
    FieldEle* pT[8];
    OperateSys *pMult = pField->pGroup2;
    OperateSys *pLus = pField->pGroup1;

    for(i=0; i<10; i++)
    {
        for(j=0; j<3; j++)
        {
            k = FakeRand(i+j*10);
            SetGenPara(pMult,k);
            pT[j] = pMult->xGen(pMult,k);
        }
        pT[3] = pLus->xOperat(pT[1],pT[2]);
        pT[4] = pMult->xOperat(pT[0],pT[3]);
        pT[5] = pMult->xOperat(pT[0],pT[1]);
        pT[6] = pMult->xOperat(pT[0],pT[2]);
        pT[7] = pLus->xOperat(pT[5],pT[6]);

        rc = pMult->xIsEqual(pT[4],pT[7]);

        for(j=0; j<8; j++)
        {
            FreeGroupEle(pLus,pT[j]);
        }

        assert( rc );
    }
    loga("Distributive ok %d",rc);
    return rc;
}

void IsField(FieldSys *pField)
{
    IsGroup(pField->pGroup1);
    IsGroup(pField->pGroup2);
    DistributiveLaw(pField);
}

void RationTest(FieldSys *pField)
{
    FieldEle *p1,*p2;
    FieldEle *pTest;
    //p1 = pField->pGroup1->xGen(1,1,2);
//    p1 = RationGen1(1,12,0);
//    p2 = RationGen1(3,8,0);
    p1 = RationGen2(pField->pGroup2,1,12,0);
    p2 = RationGen2(pField->pGroup2,3,8,0);
    pTest = pField->pGroup2->xOperat(p1,p2);
    loga("ration %d/%d",pTest->nmrtr,pTest->dnmtr);

}

