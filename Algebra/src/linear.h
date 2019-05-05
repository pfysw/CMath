/*
 * linear.h
 *
 *  Created on: Apr 23, 2019
 *      Author: Administrator
 */

#ifndef LINEAR_H_
#define LINEAR_H_
#include "type.h"


#define BASE_ELE_TYPE 0
#define FIRST_LEVEL_VEC 1
#define SECOND_LEVEL_VEC 2

struct VectorEle
{
    int nEle;
    u8 eType;//指定aVecEle元素类型
    VectorEle *pPoly;
    FieldSys *pSubField;
    void **aVecEle;
    //FieldEle **aVecEle;
};

void FreeVector(VectorEle *pVec);

VectorEle *FieldMultVector(
        FieldSys *pField,
        void *pEle,
        VectorEle *pVector1);

int isVecotrEqual(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2);

VectorEle *VectorPlus(
        FieldSys *pField,
        VectorEle *pVector1,
        VectorEle *pVector2);

VectorEle *VectorMult(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2);

VectorEle *VectorMod(
        FieldSys *pField,
        VectorEle *p1,
        VectorEle *p2,
        u8 bQuotient);
VectorEle *NewVector(int nEle);
VectorEle *NewVector1(VectorEle *pSrc,int nEle);
int SolveLinearEqu(
        FieldSys *pField,
        VectorEle **paVector,//输入的向量组
        VectorEle *pRight,//等式右边向量
        VectorEle *pSolve,
        int iCol,//起始列
        int n,//n个变量
        int iRow,//输入的方起始行
        int nRow
        );
VectorEle *VectorTran(
        FieldSys *pField,
        VectorEle *p1);

void PrintVal(
        FieldSys *pField,
        VectorEle **paVector,
        int nCol);
void PrintVec(FieldSys *pField,VectorEle *p);
#endif /* LINEAR_H_ */
