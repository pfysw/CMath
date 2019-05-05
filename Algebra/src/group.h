/*
 * group.h
 *
 *  Created on: Apr 11, 2019
 *      Author: Administrator
 */

#ifndef GROUP_H_
#define GROUP_H_
#include <stdio.h>
#include "type.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"

#define FIELD_ELE 0
#define VEC_ELE 1
struct OperateSys
{
    void *pBaseEle;
    u8 nPara;
    u8 isMult;
    u8 typeEle;//0£ºFieldEle 1£ºVectorEle
    u8 aGenPara[3];
    FieldSys *pSubFiled;
    int (*xIsEqual)(void *, void *);
    void *(*xGen)(OperateSys*,u32 iNum);
    void *(*xInvEle)(void *);
    void *(*xRecursiveGen)(void *pEle);
    void *(*xOperat)(void *, void *);
};

void SetOperaSys(OperateSys **ppOpSys);
pthread_t CreatthreadTest(OperateSys *pOpSys);
int AssociativeLaw(OperateSys *pOpSys);
int HasIdentityEle(OperateSys *pOpSys);
int HasInvEle(OperateSys *pOpSys);
void *GenSubFieldEle(OperateSys *pOpSys, int k);
void FreeGroupEle(OperateSys *pOpSys,void *p);
#endif /* GROUP_H_ */
