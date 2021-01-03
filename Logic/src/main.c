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
#include "db.h"


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

char aMemIn[100] = "(~A->~B)->(K->A);";
FILE *BindMemFd(yyscan_t scanner,char *buf, int len)
{
    FILE *fd = NULL;
    if (!(fd = fmemopen(buf, len,"r")))
    {
        printf("fopen %s error %p\n",buf,fd);
        exit(0);
    }
    else
    {
        yyset_in(fd, scanner);
    }
    return fd;
}


void Token2AstTree(
        AstParse *pParse,
        yyscan_t scanner,
        Vector *pV)
{
    TokenInfo *pToken;
    void* pLemon = PropParseAlloc(malloc);
    char *zSymb;
    int token;
    int idx = 0;

    pToken = NewNode(pParse);
    token = yylex(scanner);
    zSymb = yyget_text(scanner);
    while (token)
    {
        pToken->zSymb = zSymb;
        pToken->nSymbLen = yyget_leng(scanner);
        pToken->symb = pToken->zSymb[0];
        PropParse(pLemon, token, pToken,pParse);
        //PropParseTrace(stdout, "");
        if( token==TK_SEM )
        {
            PropParse(pLemon, 0, 0,pParse);
//           log_a("----- %d -----",idx);
//           PrintAst(pParse,pParse->pRoot);//
            idx++;
            InsertVector(pV,pParse->pRoot);
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
            if(!memcmp(zSymb,"end",3)){
                pParse->all_num = idx;
                break;
            }
            pToken = NewNode(pParse);
           continue;
        }
    }
    printf("end %d %s\n",token,yyget_text(scanner));
    PropParseFree(pLemon, free);
}

void GetPropStrParse(AstParse *pParse,char *buf,Vector *pV,int len)
{
    FILE *fd = NULL;
    yyscan_t scanner;
    yylex_init(&scanner);
    fd = BindMemFd(scanner,buf,len);
    Token2AstTree(pParse,scanner,pV);
    yylex_destroy(scanner);
    fclose(fd);
}

int main(int argc, char** argv) {

   int token;
   yyscan_t scanner;
   FILE *fd = NULL;
   TokenInfo *pToken;
   void* pLemon;
   AstParse *pParse;
  // TokenInfo *ppTest[100];
   int idx = 0;
   char *zSymb;

   setbuf(stdout, NULL);
   yylex_init(&scanner);

   fd = BindScanFd(scanner,"in.sh");
//   char *test = malloc(100);
//   strcpy(test,aMemIn);
//   fd = BindMemFd(scanner,test);//test

   pParse = CreatAstParse();
   InitTheoremSet(pParse);
#if 0
   pLemon = PropParseAlloc(malloc);
   pToken = NewNode(pParse);
   token = yylex(scanner);
   zSymb = yyget_text(scanner);
   //PropParseTrace(stdout, "");
   while (token)
   {
       pToken->zSymb = zSymb;//yyget_text(scanner);
       pToken->nSymbLen = yyget_leng(scanner);
       pToken->symb = pToken->zSymb[0];
     //  printf("tocken %d %s \n",token,pToken->zSymb);
       PropParse(pLemon, token, pToken,pParse);

       if( token==TK_SEM )
       {
           PropParse(pLemon, 0, 0,pParse);
//           log_a("----- %d -----",idx);
//           PrintAst(pParse,pParse->pRoot);//
           //ppTest[idx++] = pParse->pRoot;
           idx++;
           InsertVector(&theoremset,pParse->pRoot);
       }
	   token = yylex(scanner);
	   if( token )
	   {
	       zSymb = yyget_text(scanner);
	       //printf("zSymb %s\n",zSymb);
           if(!memcmp(zSymb,"formula",7)){
                pParse->axiom_num = idx;
                token = yylex(scanner);//jump formula
                token = yylex(scanner);//jump ;
                zSymb = yyget_text(scanner);
           }
           if(!memcmp(zSymb,"end",3)){
	           pParse->all_num = idx;
	           break;
	       }

	       pToken = NewNode(pParse);

	       continue;
	   }
   }
   printf("end %d %s\n\n",token,yyget_text(scanner));
   yylex_destroy(scanner);
   fclose(fd);
   PropParseFree(pLemon, free);
#endif
   Token2AstTree(pParse,scanner,&theoremset);
   yylex_destroy(scanner);
   fclose(fd);

   //GenBasicProp(pParse);
  // SubstPropTest(pParse,ppTest);
  // SubstSingleTest(pParse,theoremset.data);
   SubstMpTest(pParse,theoremset.data);
   for(int i=0;i<3;i++){
       FreeAstNode(pParse,pParse->apAxiom[i]);
   }
   log_a("malloc %d free %d",pParse->malloc_cnt,
           pParse->free_cnt);
#ifdef FREE_TEST
   extern u8 testbuf[10000];
   for(int i=0;i<pParse->malloc_cnt;i++){
       if(testbuf[i]){
           printf("not free %d\n",i);
       }
   }
#endif
   CloseAstParse(pParse);
   printf("%ld\n",sizeof(TokenInfo));
   return 0;
}



