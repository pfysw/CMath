/*
 * token.h
 *
 *  Created on: Aug 9, 2019
 *      Author: Administrator
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include <stdio.h>
#include "prop.lemon.h"

//#define FREE_TEST

//#define TK_SEM 100
typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

#define PROP_SYMB 1
#define PROP_NEG  2
#define PROP_IMPL 3

#define OP_IMPL 0
#define OP_MP 1
#define OP_HS 2
#define OP_ADD 3

#define log_a(format,...)   printf(format"\n",## __VA_ARGS__)
#define log_c(format,...)   printf(format,## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)

typedef struct TokenInfo TokenInfo;
struct TokenInfo{
    char *zSymb;
    int nSymbLen;
    TokenInfo *pParent;
    TokenInfo *pSubst;
    TokenInfo *pLeft;
    TokenInfo *pRight;
    TokenInfo *pTheorem;
    TokenInfo *pDeduce;
    u8 op;
    u8 type;
    u8 bSubst;
    u8 isRightTheorem;//1:右边是定理
    u8 iRight;
    u8 isDeduction:4;
    u8 isNewTemp:4;
    char symb;
    char copy;
#ifdef FREE_TEST
    int malloc_flag;
    int malloc_string;
#endif
};

#endif /* TOKEN_H_ */
