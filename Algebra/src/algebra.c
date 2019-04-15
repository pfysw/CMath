/*
 * algebra.c
 *
 *  Created on: Apr 11, 2019
 *      Author: Administrator
 */
#include"algebra.h"

void IsGroup(OperateSys *pOpSys)
{
    AssociativeLaw(pOpSys);
    HasIdentityEle(pOpSys);
    HasInvEle(pOpSys);
}

Algebra *AlgebraOpen(void)
{
	Algebra *pAlgebra = (Algebra*)malloc(sizeof(Algebra));
	memset(pAlgebra, 0, sizeof(Algebra));
	return pAlgebra;
}

