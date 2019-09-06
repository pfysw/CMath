/*
 * prop.h
 *
 *  Created on: Sep 1, 2019
 *      Author: Administrator
 */

#ifndef PROP_H_
#define PROP_H_
#include "token.h"

void GenBasicProp(AstParse *pParse);
int  SubstProp(
        AstParse *pParse,
        TokenInfo *pA,
        TokenInfo *pB);
void  SubstPropTest(
        AstParse *pParse,
        TokenInfo **ppTest);
#endif /* PROP_H_ */
