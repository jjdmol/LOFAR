%{
#include <fstream>
#include <strstream>
#include <iostream>
#include <string>
#include <typeinfo>

       //#include <stdio.h>
#include "selfparse.tab.h"

#define DEBUG(x) if(debug_flag_on) std::cout << x << std::endl;
       //#define DEBUG(x) if(debug_flag_on) fprintf(stdout,"%s\n",x);
       //bool debug_flag_on(true);
   int debug_flag_on = 1;

#ifdef __cplusplus
extern "C" {
#endif

   int selfparseparse();

#ifdef __cplusplus
}
#endif

       //#define lexerwrap yywrap

       //yyFlexLexer lexi;

       //#define yylex() lexi.yylex()

unsigned long lineno(0);

%}
%option yylineno
%option debug
%option noyywrap


digit    [0-9]
number   [0-9]+
id       [a-z][a-z0-9]*
branch   start(\.[0-9]+)+

ws       [ \t]+
nl       \n
lbrace   \{
rbrace   \}
lbracket \[
rbracket \]
comma    ,
minussign -

part_token "part"
peeling "peeling" 
division "devide"
brute_force "brute_force"
script_token "calibrate_observation"
competition "competition"
collaboration "collaboration"
frequency frequency
time time
baseline baseline
source source
field field

%%

{peeling}       { DEBUG("peeling-command"); return PEELING; }

{division}      { DEBUG("division-command"); return DIVISION; }

{brute_force}      { DEBUG("brute_force-command"); return BRUTE_FORCE; }

{script_token} { DEBUG("-- calibration --"); return SCRIPT_TOKEN; }

{competition} { DEBUG("competition-token"); return COMPETITION; }

{collaboration} { DEBUG("collaboration-token"); return COLLABORATION; }

{part_token} { DEBUG("part: " << yytext ); return PART_TOKEN; }

{frequency} { DEBUG("frequency :"); return FREQUENCY; }
{time} { DEBUG("time :"); return TIME; }
{baseline} { DEBUG("baseline :"); return BASELINE; }
{source} { DEBUG("source :"); return SOURCE; }
{field} { DEBUG("field :"); return FIELD; }


{number} {  DEBUG("number: " << yytext); return NUMBER; }

{minussign} { DEBUG(" upto ");  return MINUSSIGN; }

{branch} { DEBUG("branch " << yytext); return BRANCH; }

{comma}     { DEBUG(" and "); return COMMA; }

{lbracket}  { DEBUG("["); return LBRACKET; }

{rbracket}  { DEBUG("]"); return RBRACKET; }

{lbrace}    { DEBUG("{"); return LBRACE; }

{rbrace}    { DEBUG("}"); return RBRACE; }

{ws} { /* DEBUG("änother shitloadf of spilled binairity"); */ }


%%

FlexLexer * lexi;

int main(int ac, char ** av )
{
   lexi = new yyFlexLexer();
   ++av, --ac;  // skip over program name
   if ( ac > 0 )
      do
      {
         std::istream *yyin = new std::ifstream(av[0]);
             //yyin = fopen(av[0],"r"); 
         lexi->switch_streams(yyin,NULL);
         selfparseparse();
             //         (std::ifstream*)(yyin)->close();
         delete yyin;
         ++av;
      }while(--ac > 0);
   else
   {
      selfparseparse();
   }
   return 0;
}

std::string progname ("selfcalparse");
#ifdef __cplusplus
extern "C"
{
#endif
   int selfparseerror(char * s)
   {
      std::cerr << progname << ":" << lineno << ":" << s << std::endl;
      return 0;
   }

       //int selfparselex(YYSTYPE *yylval)
   int selfparselex()
   {
                //   std::istream *yyin = new std::istrstream((char * ) (yylval));
       //   lexi->switch_streams(yyin,NULL);
      return lexi->yylex();
   }

#ifdef __cplusplus
}
#endif
