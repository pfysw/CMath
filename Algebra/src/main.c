/*
 * main.c
 *
 *  Created on: Apr 9, 2019
 *      Author: Administrator
 */
#include<stdio.h>
#include"algebra.h"
#include "group.h"

void test(void);

int main(void)
{
	Algebra *pAlgebra = AlgebraOpen();

	setvbuf(stdout, NULL, _IONBF, 0);

	PermPrintTest(pAlgebra->pOpSys);
	FieldTest(pAlgebra->pField);
//	test();
//	loga("dd");
//	sleep(1);

	return 0;
}


typedef struct TT TT;
struct TT
{
    int nEle;
};

//typedef struct TT1 TT1;
//struct TT1
//{
//    TT *pTT;
//};
//
//
//void test1(TT (*aaa)[10])
//{
//    loga("dd");
//    loga("%x %x",aaa[1][1].nEle,aaa[1][8].nEle);
//}
//void test(void)
//{
//    TT (*aaa)[10];
//    TT kk[10][10];
//    TT1 *pp[10];
//    int i;
//    kk[1][1].nEle = 7;
//    kk[1][8].nEle = 5;
//
//    aaa = kk;
//    for(i=0; i<10; i++)
//    {
//        pp[i] = malloc(sizeof(TT1));
//        pp[i]->pTT = kk[i];
//    }
//    loga("%x %x",&kk[1][1],&kk[1][8]);
//    loga("%x %x",&pp[1]->pTT[1] ,kk[1]);
//   // test1(kk);
//   // loga("%x %x",aaa[1][1].nEle,aaa[1][8]);
//   // loga("%d %d",aaa[1][1]->nEle,aaa[1][8]->nEle);
//   // loga("%d %d",kk[1][1].nEle,kk[1][8].nEle);
//}

