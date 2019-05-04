/*
 * algebra.c
 *
 *  Created on: Apr 11, 2019
 *      Author: Administrator
 */
#include"algebra.h"
#include"group.h"

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
	SetOperaSys(&pAlgebra->pOpSys);
	SetFieldSys(&pAlgebra->pField);
	SetVecField(&pAlgebra->pVecField,pAlgebra->pField,3);
	return pAlgebra;
}

void AlgebraTest(Algebra *pAlgebra)
{
    int rc = 4;
    FieldSys *pField = pAlgebra->pField;
    FieldSys *pVecField = pAlgebra->pVecField;
    loga("field");
    IsField(pField);
    loga("vector");
    VectorTest(pField);
    PolyTest(pVecField);
//    IsGroup(pField->pGroup1);
//    IsGroup(pField->pGroup2);
}

