%{

#include <stdio.h>
#include "selfparse.tab.h"

       //#define DEBUG(x) if(debug_flag_on) std::cout << x << std::endl;
#define DEBUG(x) if(debug_flag_on) fprintf(stdout,"%s\n",x);
       //bool debug_flag_on(true);
       //static int debug_flag_on = 1;

#include "parser/selfparse.h"

unsigned long long int line_no = 0;

#define YY_INPUT(buf,result,max_size) result = selfparseGetChars(buf,max_size);

%}
/*
%option debug
%option verbose
%option reentrant
*/
%option yylineno
%option noyywrap

digit     [0-9]
number    [0-9]+
real      [0-9]+\.[0-9]+
id        [a-z][a-z0-9]*
branch    start(\.[0-9]+)+
ref       script(\.[A-Za-z][0-9A-Za-z_]*)+
refbranch script(\.[A-Za-z][0-9A-Za-z_]*)+(\.[0-9]+)+

ws       [ \t]+
nl       \n
lbrace   \{
rbrace   \}
lbracket \[
rbracket \]
comma    ,
rangesign \:
minussign -
plussign \+
timessign \*
devidesign \/
powersign \^
c_comment_start "/*"
c_comment_end "*/"
CC_comment_start "//"
part_token "part"
command "peeling"|"devide"|"brute_force"
script_token "calibrate_observation"
competition "competition"
collaboration "collaboration"
dimension "frequency"|"time"|"baseline"|"source"|"field"

%x c_comment
%x CC_comment

%%

{script_token}             { return SCRIPT_TOKEN; }

{competition}              { return COMPETITION; }

{collaboration}            { return COLLABORATION; }

{part_token}               { return PART_TOKEN; }

{command}                  {
   selfparselvalp->command = strdup(yytext);
   return COMMAND;
}

{dimension}                {
   selfparselvalp->dimension = strdup(yytext);
   return DIMENSION;
}

{number}                   {
   selfparselvalp->number = atoi( yytext );
   return NUMBER;
}

{real}                     {
   selfparselvalp->real = atof( yytext );
   return REAL;
}

{rangesign}                { return RANGESIGN; }

{minussign}                { return MINUSSIGN; }

{plussign}                 { return PLUSSIGN; }

{timessign}                { return TIMESSIGN; }

{devidesign}               { return DEVIDESIGN; }

{powersign}                { return POWERSIGN; }

{branch}                   {
   ECHO;
   selfparselvalp->branchId = strdup(yytext);
   return BRANCH;
}

{ref}                   {
   ECHO;
   selfparselvalp->branchId = strdup(yytext);
   return REF;
}

{refbranch}                   {
   ECHO;
   selfparselvalp->branchId = strdup(yytext);
   return REF;
}

{comma}                    { return COMMA; }

{lbracket}                 { return LBRACKET; }

{rbracket}                 { return RBRACKET; }

{lbrace}                   { return LBRACE; }

{rbrace}                   { return RBRACE; }

{ws}

{nl}                       { line_no++;}

<c_comment>[^*\n]*         { /* eat anything that's not a '*' */ }
<c_comment>"*"+[^/\n]*     { /* eat up '*'s not followed by '/'s */ }
<c_comment>{nl}            { ++line_no; }
<c_comment>{c_comment_end} BEGIN(INITIAL);

{c_comment_start}          BEGIN(c_comment);

{CC_comment_start}         BEGIN(CC_comment);

<CC_comment>[^\n]*         BEGIN(INITIAL); /* eat up to and not including \n and start all over */

.                          ECHO;

%%
