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

//#define TK_SEM 100
typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

#define PROP_SYMB 1
#define PROP_NEG  2
#define PROP_IMPL 3

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
    u8 type;
    u8 bSubst;
    u8 isRightTheorem;//1:右边是定理
    u8 iRight;
    char symb;
    char copy;
};

typedef struct AstParse AstParse;
struct AstParse
{
    TokenInfo *pRoot;
    u8 bDiscard;
    int n;
    int malloc_cnt;
    int free_cnt;
};

void PrintAst(AstParse *pParse,TokenInfo *pAst);
void SetSymb(AstParse *pParse,TokenInfo *pB);
TokenInfo *NewNode(AstParse *pParse);
void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB);
void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC);
void FreeAstNode(AstParse *pParse,TokenInfo *p);
void FreeAstTree(AstParse *pParse,TokenInfo **ppAst,TokenInfo **ppTemp);
void PrintAstAddr(AstParse *pParse,TokenInfo *pAst);
void PrintSubstAst(AstParse *pParse,TokenInfo *pAst);
TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst);

#endif /* TOKEN_H_ */
