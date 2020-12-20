/*
 * ast.h
 *
 *  Created on: Dec 13, 2020
 *      Author: Administrator
 */

#ifndef AST_H_
#define AST_H_
#include "token.h"

typedef struct AstParse AstParse;
struct AstParse
{
    TokenInfo *pRoot;
    TokenInfo * apAxiom[3];
    TokenInfo **ppTemp;//存在递归时的共享变量
    u8 bDiscard;
    int n;
    int axiom_num;
    int all_num;
    int malloc_cnt;
    int free_cnt;
    int test;
};

void PrintAst(AstParse *pParse,TokenInfo *pAst);
void SetSymb(AstParse *pParse,TokenInfo *pB);
TokenInfo *NewNode(AstParse *pParse);
void SetNegExpr(AstParse *pParse,TokenInfo *pA, TokenInfo *pB);
void SetImplExpr(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB,
        TokenInfo *pC,
        TokenInfo *pD);
void FreeAstNode(AstParse *pParse,TokenInfo *p);
void FreeAstTree( AstParse *pParse,TokenInfo **ppAst,TokenInfo **ppTemp);
void PrintAstAddr(AstParse *pParse,TokenInfo *pAst);
void PrintSubstAst(AstParse *pParse,TokenInfo *pAst);
TokenInfo *CopyAstTree(
        AstParse *pParse,
        TokenInfo *pSrc,
        u8 bSubst);
AstParse *CreatAstParse(void);
TokenInfo * NewImplyNode(
        AstParse *pParse,
        TokenInfo *pB,
        TokenInfo *pC,
        char *zSymb);
void FreeNewImplyNodes(AstParse *pParse,TokenInfo **ppAst);
TokenInfo * NewNegNode(AstParse *pParse,TokenInfo *pB);

#endif /* AST_H_ */
