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

	setvbuf(stdout, NULL, _IONBF, 0);

	PermPrintTest(pAlgebra->pOpSys);
	AlgebraTest(pAlgebra);

	return 0;
}




