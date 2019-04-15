/*
 * util.h
 *
 *  Created on: Apr 14, 2019
 *      Author: Administrator
 */

#ifndef UTIL_H_
#define UTIL_H_
#include"type.h"

#define loga(format,...)   printf(format"\n",## __VA_ARGS__)

u32 FakeRand(int n);
void memout(u8 *pdata,int len);

#endif /* UTIL_H_ */
