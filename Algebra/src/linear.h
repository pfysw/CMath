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
    u8 eType;//ָ��aVecEleԪ������
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
        VectorEle **paVector,//�����������
        VectorEle *pRight,//��ʽ�ұ�����
        VectorEle *pSolve,
        int iCol,//��ʼ��
        int n,//n������
        int iRow,//����ķ���ʼ��
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
