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

struct OperateSys
{
    void *pBaseEle;
    u8 nPara;
    u8 isMult;
    u8 aGenPara[3];
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

#endif /* GROUP_H_ */
