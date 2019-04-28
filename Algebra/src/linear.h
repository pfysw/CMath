/*
 * linear.h
 *
 *  Created on: Apr 23, 2019
 *      Author: Administrator
 */

#ifndef LINEAR_H_
#define LINEAR_H_
#include "type.h"

struct VectorEle
{
    int nEle;
    VectorEle *pPoly;
    FieldSys *pField;
    FieldEle **aVecEle;
};

void FreeVector(VectorEle *pVec);
#endif /* LINEAR_H_ */
