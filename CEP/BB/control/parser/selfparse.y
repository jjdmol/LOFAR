%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parser/selfparse.h"

int partnum = 0;

char * branch = ("start");
unsigned int subBranch = (0);
    // int yylex(YYSTYPE*);

int topLevel  = 1; /* true that is */
 unsigned int level = 0;
int topScript = 1; /* first token indicates script */

int report(char * s);

char * calculateBrancheNumber();

%}

%debug
%defines
    //%destructor
    //%locations
%verbose

%union {
      long int number;
      float real;
      char *branchId;
      char *block;
      char *command;
      char *options;
      char *param;
      char *dimension;
      char *subScript;
}

%token <branchId> BRANCH
%token <number> NUMBER
%token <real> REAL
%token LBRACE
%token RBRACE
%token COMMA
%token RANGESIGN
%token MINUSSIGN
%token PLUSSIGN
%token TIMESSIGN
%token DEVIDESIGN
%token LBRACKET
%token RBRACKET
%token PART_TOKEN
%token <command> COMMAND
%token <command> PEELING
%token <command> DIVISION
%token <command> BRUTE_FORCE
%token SCRIPT_TOKEN
%token COMPETITION
%token COLLABORATION
%token <dimension> DIMENSION
%token <dimension> FREQUENCY
%token <dimension> TIME
%token <dimension> SOURCE
%token <dimension> FIELD
%token <dimension> BASELINE

%pure-parser

%start script

%type <block> script part_block parts part control_block control
%type <block> defining_part referencing_part

%type <branchId> topbranch

%type <options> param_block

%type <command> commandname

%type <subScript> definition_part

%type <param> range

%type <param> tactic_param

%type <param> tactic_params

%type <real> exp

%type <dimension> dimension

%defines

%left NEG     /* negation--unary minus */

%left MINUSSIGN
%left PLUSSIGN
%left TIMESSIGN
%left DEVIDESIGN
%left POWERSIGN
%left BRANCH
%%

script :
         SCRIPT_TOKEN control_block {
          printf("\nscript parsed:\n%s\nend of script\n",$2);
          saveScript("callibrate_observation", $2);
       }
       | topbranch {
          branch = $1;
          topScript = 0;
       }
       definition_part {
          printf("\nbranch-parsed:\n%s\ndefinition-part:\n%s\nend of parsed branch\n",$1,$3);
          saveScript($1, $3);
       }
       | { selfparseerror(" no script found "); }
       ;

part : defining_part {$$ = $1; }
     | referencing_part { $$ = $1; }
     ;

defining_part : PART_TOKEN definition_part {
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
   fprintf(stderr,"defining-part-parsed:\n%s\nend of defining part",$2);
   free ($2);
}
              ;

definition_part : commandname control_block {
                   report("parameterless command block");
                   if(level == 1)
                   {
                      char * bn = strdup(calculateBrancheNumber());
                      fprintf(stderr,"\ndefinition-commandname:\n%s\ncontrol block:\n%s\nend of definition part", $1, $2);
                      saveSubScript(bn, $1 , NULL , $2 );
                      $$ = bn;
                   }
                   else
                   {
                      $$ = (char*)(malloc(strlen($1) + strlen($2) + 4));
                      sprintf($$,"%s\n%s\n", $1, $2);
                      free($1);
                      free($2);
                   }
                }
                | commandname param_block control_block {
                   report("parameter command block");
                   if(level == 1) /* replace and save the nested part */
                   {
                      char * bn = strdup(calculateBrancheNumber());
                      fprintf(stderr,"\ndefinition-commandname:\n%s\nparameters:\n%s\ncontrol:\n%s\nend of definition part\n", $1, $2, $3);
                      saveSubScript(bn, $1 , $2 , $3 );
                      $$ = bn;
                   }
                   else /* just copy it */
                   {
                      $$ = (char*)(malloc(strlen($1) + strlen($2) + strlen($3) + 5));
                      sprintf($$,"%s %s\n%s\n", $1, $2, $3);
                      free($1);
                      free($2);
                      free($3);
                   }
                }
;

referencing_part : PART_TOKEN BRANCH {
   printf("referencing part parsed: %s", $2);
   $$ = (char*)(malloc(strlen($2) + 8));
   sprintf($$,"part %s\n", $2);
}
                 ;

param_block : LBRACKET dimension tactic_params RBRACKET {
   $$ = (char*)(malloc(strlen($2)+strlen($3)+5));
   sprintf($$,"[%s %s]",$2,$3);
   fprintf(stderr,"param-block:\n%s\nend of parameter block\n",$$);
   free($3);
}
             ;

tactic_params : tactic_param { $$ = $1; }
              | tactic_param COMMA tactic_params {
                 $$ = (char*)(malloc(strlen($1)+strlen($3)+2));
                 sprintf($$,"%s,%s",$1,$3);
                 free($1);
                 free($3);
}
              ;

tactic_param : exp   {
                 int seiz=20;
                 $$ = (char*)(malloc(seiz));
                 seiz = snprintf($$,seiz,"%f",$1);
                        /* <todo>check if it fits</todo> */
                 if(seiz >= 20)
                 {
                    report("double didn't fit in slot.");
                 }
}
             | range { $$ = $1; }
             | error { $$ = (char*)(malloc(2)); strcpy($$,"\0");}
             ;

range : exp RANGESIGN exp {
 /*<todo>strcat it into this string</todo>*/
   $$ = (char*)(malloc(44));
   sprintf($$,"%g:%g",$1,$3);
}
      ;

exp : NUMBER               { $$ = $1;         }
        | REAL                { $$ = $1;         }
    | exp PLUSSIGN exp                 { $$ = $1 + $3;    }
    | exp MINUSSIGN exp                 { $$ = $1 - $3;    }
    | exp TIMESSIGN exp                 { $$ = $1 * $3;    }
    | exp DEVIDESIGN exp                 { $$ = $1 / $3;    }
    | MINUSSIGN exp  %prec NEG          { $$ = -$2;        }
    | exp POWERSIGN exp                 { $$ = pow ($1, $3); }
    | '(' exp ')'                 { $$ = $2;         }

part_block : LBRACE parts RBRACE {
   $$ = (char*)(malloc(strlen($2) + 4));
   sprintf($$,"{%s}\n",$2);
   free($2);
}
           ;

control_block : LBRACE control RBRACE {
   $$ = (char *)(malloc(strlen($2) + 4));
   sprintf($$,"{%s}\n",$2);
   free($2);
}
              | LBRACE RBRACE { $$ = (char *)(malloc(4)); sprintf($$,"{}\n"); }
              ;

control : COMPETITION { level ++; } part_block  {
   $$ = (char *)(malloc(strlen($3) + 3 + 11));/*strlen("competition")*/
   sprintf($$,"competition %s\n",$3);
   free($3);
   level--;
}
        | COLLABORATION { level ++; } part_block {
   $$ = (char*)(malloc(strlen($3) + 3 + 13));/*strlen("collaboration")*/
   sprintf($$,"collaboration %s\n",$3);
   free($3);
   level--;
}
        ;

parts : part { $$ = $1; }
      | part parts {
   $$ = (char *)(malloc(strlen($1) + strlen($2) + 2));
   sprintf($$,"%s\n%s",$1,$2);
   free($1);
   free($2);
}
      ;


commandname : COMMAND { $$ = $1; }
            ;

dimension : DIMENSION { $$ = $1; }
          ;

topbranch: BRANCH { $$ = $1;}
      ;
%%

int report(char * s)
{
       // if (debug_or_verbose)
   return fprintf(stdout,s);
}

char * calculateBrancheNumber()
{
   static char * branchNumber = 0;
   char subBstr[6];
   int rc = snprintf(subBstr, 5, "%u", subBranch++);
   if( rc > 5 )
   {
      report ("branch number truncated");
   }
   if(branchNumber)
   {
      free(branchNumber);
   }
       //   branchNumber = new char [ strlen(branch) + strlen(subBstr) + 2];
   branchNumber = (char *)(malloc( strlen(branch) + strlen(subBstr) + 2));

   sprintf(branchNumber,"%s.%s",branch,subBstr);

   return branchNumber;
}


void saveSubScript(char * bn, char * command, char * options , char * block)
{
   FILE * of;
   char * ext = ".selfcal";
   char * ofname = (char*)(malloc(strlen(bn) + strlen(ext) + 1));
   strcpy(ofname,bn);
   strcat(ofname,ext);
   of = fopen(ofname,"w");
   if(options)
   {
      fprintf(of,"%s %s %s %s", bn, command, options, block);
   }
   else
   {
      fprintf(of,"%s %s %s", bn, command, block);
   }
   fclose(of);
   free(ofname);
}

void saveScript( char * command, char * block)
{
   FILE * of;
   char * ext = ".out";
   char * ofname = (char*)(malloc(strlen(filename) + strlen(ext) + 1));
   strcpy(ofname,filename);
   strcat(ofname,ext);
   of = fopen(ofname,"w");
   fprintf(of,"%s %s", command, block);
   fclose(of);
   free(ofname);
}
