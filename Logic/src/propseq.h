/*
 * propseq.h
 *
 *  Created on: Dec 17, 2020
 *      Author: Administrator
 */

#ifndef PROPSEQ_H_
#define PROPSEQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include <assert.h>
#include "prop.h"

//#define MP_DEBUG

TokenInfo * PropAdd(AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq);
TokenInfo * PropRemoveAdd(
        AstParse *pParse,
        TokenInfo **ppTest,
        TokenInfo *pSeq);

#endif /* PROPSEQ_H_ */
