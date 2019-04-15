/*
 * main.c
 *
 *  Created on: Apr 9, 2019
 *      Author: Administrator
 */
#include<stdio.h>
#include"algebra.h"
#include "group.h"

int main(void)
{
	Algebra *pAlgebra = AlgebraOpen();
	OperateSys *pOpSys;
	pthread_t t1;

	setvbuf(stdout, NULL, _IONBF, 0);

	SetOperaSys(&pAlgebra->pOpSys);
	pOpSys = pAlgebra->pOpSys;
	t1 = CreatthreadTest(pAlgebra->pOpSys);
	//PermPrintTest(pAlgebra->pOpSys);
	pthread_join(t1,NULL);

	return 0;
}
