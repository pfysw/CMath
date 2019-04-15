/*
 * algebra.h
 *
 *  Created on: Apr 11, 2019
 *      Author: Administrator
 */

#ifndef ALGEBRA_H_
#define ALGEBRA_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "type.h"
#include "util.h"

typedef struct Algebra Algebra;

struct Algebra
{
    OperateSys *pOpSys;
};

Algebra *AlgebraOpen(void);
void IsGroup(OperateSys *pOpSys);
#endif /* ALGEBRA_H_ */
