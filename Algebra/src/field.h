/*
 * field.h
 *
 *  Created on: Apr 16, 2019
 *      Author: Administrator
 */

#ifndef FIELD_H_
#define FIELD_H_
#include "type.h"


#define ROW 5
#define COL 3

typedef struct FieldEle FieldEle;
struct FieldEle
{
    u32 nmrtr;//分子
    u32 dnmtr;//分母
    float val;
    u8 eSymb;
};

typedef struct ModEle ModEle;
struct ModEle
{
    u32 num;
    u32 mod;
};

struct FieldSys
{
    OperateSys *pGroup1;//加法群
    OperateSys *pGroup2;//乘法群
    FieldSys *pParent;
    VectorEle **paVector;
};

OperateSys *RationPlusObj(void);
void SetFieldSys(FieldSys **ppField);
void SetGenPara(OperateSys* pOpSys, u32 iNum);

#endif
