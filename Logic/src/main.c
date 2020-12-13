/*
 * main.c
 *
 *  Created on: Aug 9, 2019
 *      Author: Administrator
 */
#include <stdio.h>
#include <assert.h>
#include "lex.yy.h"
#include "ast.h"
#include "prop.h"

extern Vector theoremset;

void* PropParseAlloc(void* (*allocProc)(size_t));
void* PropParse(void*, int, TokenInfo*,AstParse *pParse);
void* PropParseFree(void*, void(*freeProc)(void*));

FILE *BindScanFd(yyscan_t scanner,char *name)
{
    FILE *fd = NULL;
    if (!(fd = fopen(name, "r")))
    {
        printf("fopen %s error %p\n",name,fd);
        exit(0);
    }
    else
    {
        yyset_in(fd, scanner);
    }
    return fd;
}

int main(int argc, char** argv) {

   int token;
   yyscan_t scanner;
   FILE *fd = NULL;
   TokenInfo *pToken;
   void* pLemon = PropParseAlloc(malloc);
   AstParse *pParse;
  // TokenInfo *ppTest[100];
   int idx = 0;
   char *zSymb;

   setbuf(stdout, NULL);
   yylex_init(&scanner);

   InitTheoremSet();
   fd = BindScanFd(scanner,"in");
//   pParse = (AstParse *)malloc(sizeof(AstParse));
//   memset(pParse,0,sizeof(AstParse));
   pParse = CreatAstParse();
   pToken = NewNode(pParse);
   token = yylex(scanner);
   zSymb = yyget_text(scanner);
   //PropParseTrace(stdout, "");
   while (token)
   {
       pToken->zSymb = zSymb;//yyget_text(scanner);
       pToken->nSymbLen = yyget_leng(scanner);
       pToken->symb = pToken->zSymb[0];
       //printf("tocken %d %s \n",token,pToken->zSymb);
       PropParse(pLemon, token, pToken,pParse);

       if( token==TK_SEM )
       {
           PropParse(pLemon, 0, 0,pParse);
//           log_a("----- %d -----",idx);
//           PrintAst(pParse,pParse->pRoot);
           //ppTest[idx++] = pParse->pRoot;
           idx++;
           InsertVector(&theoremset,pParse->pRoot);
       }
	   token = yylex(scanner);
	   if( token )
	   {
	       zSymb = yyget_text(scanner);
           if(!memcmp(zSymb,"formula",7)){
                pParse->axiom_num = idx;
                token = yylex(scanner);//jump formula
                token = yylex(scanner);//jump ;
                zSymb = yyget_text(scanner);
           }
           else if(!memcmp(zSymb,"end",3)){
               assert(pToken->symb==';');
	           pParse->all_num = idx;
	           break;
	       }

	       pToken = NewNode(pParse);

	       continue;
	   }
   }
   printf("end %d %s\n\n",token,yyget_text(scanner));

   //GenBasicProp(pParse);
  // SubstPropTest(pParse,ppTest);
   //SubstSingleTest(pParse,theoremset.data);
   SubstMpTest(pParse,theoremset.data);
   for(int i=0;i<3;i++){
       FreeAstNode(pParse,pParse->apAxiom[i]);
   }
   yylex_destroy(scanner);
   PropParseFree(pLemon, free);
   fclose(fd);
   //FreeAstTree(pParse,&pParse->pRoot,ppTemp);
   log_a("malloc %d free %d",pParse->malloc_cnt,
           pParse->free_cnt);

   return 0;
}



