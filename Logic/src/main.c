/*
 * main.c
 *
 *  Created on: Aug 9, 2019
 *      Author: Administrator
 */
#include<stdio.h>
#include<assert.h>
#include "lex.yy.h"
#include "token.h"
#include "prop.h"


void* PropParseAlloc(void* (*allocProc)(size_t));
void* PropParse(void*, int, TokenInfo*,AstParse *pParse);
void* PropParseFree(void*, void(*freeProc)(void*));

int main(int argc, char** argv) {

   int token;
   yyscan_t scanner;
   FILE *fd;
   TokenInfo *pToken;
   void* pLemon = PropParseAlloc(malloc);
   AstParse *pParse;
   int end_flag = 0;
   TokenInfo *ppTemp[100];
   TokenInfo *ppTest[10];
   int idx = 0;

   setbuf(stdout, NULL);
   yylex_init(&scanner);

   if (!(fd = fopen("in", "r"))) {
       perror(argv[1]);
       return 1;
   }
   else
   {
       yyset_in(fd, scanner);
   }
   pParse = (AstParse *)malloc(sizeof(AstParse));
   memset(pParse,0,sizeof(AstParse));
   pToken = NewNode(pParse);
   token = yylex(scanner);
   //ParseTrace(stdout, "");
   while (token)
   {

       pToken->zSymb = yyget_text(scanner);
       pToken->nSymbLen = yyget_leng(scanner);
       pToken->symb = pToken->zSymb[0];
       //printf("tocken %d %s \n",token,pToken->zSymb);
       PropParse(pLemon, token, pToken,pParse);
       end_flag = 0;
       if( token==TK_SEM )
       {
           end_flag = 1;
           PropParse(pLemon, 0, 0,pParse);
           log_a("----------");
           PrintAst(pParse,pParse->pRoot);
           ppTest[idx++] = pParse->pRoot;
       }
	   token = yylex(scanner);
	   if( token )
	   {
	       pToken = NewNode(pParse);
	       continue;
	   }
   }
   if( !end_flag )
   {
       PropParse(pLemon, 0, 0,pParse);
   }
   printf("end %d %s\n",token,yyget_text(scanner));

   SubstPropTest(pParse,ppTest);

   //GenBasicProp(pParse);
   yylex_destroy(scanner);
   PropParseFree(pLemon, free);

   //FreeAstTree(pParse,&pParse->pRoot,ppTemp);
   log_a("malloc %d free %d",pParse->malloc_cnt,
           pParse->free_cnt);

   return 0;
}
