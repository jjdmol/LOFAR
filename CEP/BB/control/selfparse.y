%{

#include <stdio.h>

int partnum = 0;

    //int yylex(YYSTYPE*);

%}

%union {
      long int number;
      char *branchId;
      char *block;
      char *command;
      char *dimension;
}

%token <branchId> BRANCH
%token <number> NUMBER
%token LBRACE
%token RBRACE
%token COMMA
%token MINUSSIGN
%token LBRACKET
%token RBRACKET
%token PART_TOKEN
%token <command> PEELING
%token <command> DIVISION
%token <command> BRUTE_FORCE
%token SCRIPT_TOKEN
%token COMPETITION
%token COLLABORATION
%token <dimension> FREQUENCY
%token <dimension> TIME
%token <dimension> SOURCE
%token <dimension> FIELD
%token <dimension> BASELINE

%pure-parser

%start script

%type <block> script part_block parts part control_block control
%type <block> defining_part referencing_part param_block

%defines


%%

script : SCRIPT_TOKEN control_block { report("script parsed"); }
       | BRANCH definition_part { report("part parsed"); } /* */
       | { selfparseerror(" no script found "); }
       ;

part : defining_part
     | referencing_part
     ;

defining_part : PART_TOKEN definition_part { report("defining part parsed"); }
              ;

definition_part : command control_block
                | command param_block control_block
                ;

referencing_part : PART_TOKEN BRANCH { report("referencing part parsed"); }
                 ;

param_block : LBRACKET dimension tactic_params RBRACKET {  }
             ;

tactic_params : tactic_param
              | tactic_param COMMA tactic_params
              ;

tactic_param : NUMBER
             | range
             ;

range : NUMBER MINUSSIGN NUMBER
      ;

part_block : LBRACE parts RBRACE
           ;

control_block : LBRACE control RBRACE
              ;

control : COMPETITION part_block
        | COLLABORATION part_block
        |
        ;

parts : part
      | part parts
      ;


command : PEELING
        | DIVISION
        | BRUTE_FORCE
        |
        ;

dimension : FREQUENCY
          | TIME
          | SOURCE
          | FIELD
          | BASELINE
          ;

%%

int report(char * s)
{
       // if (debug_or_verbose)
   fprintf(stdout,s);
}
