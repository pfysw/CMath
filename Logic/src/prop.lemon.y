%include {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "token.h"
}


%token_type { TokenInfo* }
%extra_argument {AstParse *pParse}
%name PropParse

%left TK_IMPL.
%left TK_NEG.

%syntax_error {
printf(" Syntax error!\n");
exit(0);
}

program ::= expr(A) TK_SEM(B). {
    pParse->pRoot = A;
    FreeAstNode(pParse,B);
	printf(" result!\n");
	PrintAst(pParse,A);
}

expr(A) ::= expr(B) TK_IMPL(D) expr(C). {
	A = NewNode(pParse);
	SetImplExpr(pParse,A,B,C);
	FreeAstNode(pParse,D);
}
expr(A) ::= TK_LPAREN(C) expr(B) TK_RPAREN(D). {
	A = B;
	FreeAstNode(pParse,C);
	FreeAstNode(pParse,D);
}
expr(A) ::= TK_NEG(C) expr(B). {
	A = NewNode(pParse);
	SetNegExpr(pParse,A,B);
	FreeAstNode(pParse,C);
	
}
expr(A) ::= TK_SYM(B). { 
	A = B;
	SetSymb(pParse,B);
	//PrintAst(pParse,A);
}
