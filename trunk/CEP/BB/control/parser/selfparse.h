#ifndef SELFPARSE_H_INCLUDED
#define SELFPARSE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

   int selfparseerror(char * s);
   int selfparseparse();

#define YY_DECL   int selfparselex(YYSTYPE* selfparselvalp)

   void saveSubScript(char * bn, char * command, char * options , char * block);
   void saveScript(char * command, char * block);

#ifdef __cplusplus
}
#endif

extern char * filename;

       //#define lexerwrap yywrap

       //yyFlexLexer lexi;

       //#define yylex() lexi.yylex()

#endif // SELFPARSE_H_INCLUDED
