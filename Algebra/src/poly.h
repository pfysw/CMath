/*
 * poly.h
 *
 *  Created on: Apr 27, 2019
 *      Author: Administrator
 */

#ifndef POLY_H_
#define POLY_H_

void SetVecField(
        FieldSys **ppField,
        FieldSys *pSubField,
        int nEle);

typedef struct MapEle MapEle;
struct MapEle
{
    void (*xSigma)(VectorEle* pSrc);
    void (*xTau)(VectorEle* pSrc);
    u8 nSigma;
    u8 nTau;
};

void SigmaMap(VectorEle* pSrc);
#endif /* POLY_H_ */
