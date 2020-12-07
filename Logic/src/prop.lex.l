%{
    #include "token.h"
    #define ECHO
%}


%option reentrant
%option noyywrap


%%

[a-zA-Z]+    { return TK_SYM; }
[0-9]+    { return TK_SYM; }
~    { return TK_NEG; }
->    { return TK_IMPL; }
>>    { return TK_IMPL; }
\+    { return TK_IMPL; }
">"    { return TK_IMPL; }
\(     { return TK_LPAREN; }
\)    { return TK_RPAREN; }
;    { return TK_SEM; }
"//".*                                    { /* DO NOTHING */ }

%%
