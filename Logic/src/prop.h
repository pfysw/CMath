/*
 * prop.h
 *
 *  Created on: Sep 1, 2019
 *      Author: Administrator
 */

#ifndef PROP_H_
#define PROP_H_
#include "ast.h"

extern int axiom_num;

typedef struct vector
{
    TokenInfo **data;
    int n;
    int size;
}Vector;

void GenBasicProp(AstParse *pParse);
int  SubstProp(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB);
void  SubstMpTest(AstParse *pParse,TokenInfo **ppTest);
void  SubstPropTest(
        AstParse *pParse,
        TokenInfo **ppTest);
TokenInfo *  PropMpSubst(
        AstParse *pParse,
        TokenInfo *pA,//条件
        TokenInfo *pB);//定理
TokenInfo * PropMpSeq(AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq);
void FreePropSeq(AstParse *pParse,TokenInfo *pSeq,TokenInfo **ppTemp);
void  SubstSingleTest(AstParse *pParse,TokenInfo **ppTest);
void InitTheoremSet(AstParse *pParse);
void InsertVector(Vector *pV,TokenInfo *pData);
int SetSameNode(
        AstParse *pParse,
        TokenInfo **ppAst,
        TokenInfo **ppTemp);
int isChildProp(AstParse *pParse,TokenInfo *pProp,TokenInfo *pSym);

#endif /* PROP_H_ */
