/*
//# KeyParse.y: Parser for key=value line
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$
*/

%{
#include <Blob/KeyParser.h>
#include <Blob/KeyValueMap.h>
using namespace LOFAR;
%}

%pure_parser                /* make parser re-entrant */

%union {
LOFAR::KeyValue* val;
LOFAR::KeyValueMap* block;
LOFAR::vector<LOFAR::KeyValue>* vec;
}

%{
int KeyTokenizelex (YYSTYPE*);
#define KeyParselex KeyTokenizelex
%}


%token <val> LITERAL
%token <val> NAME
%token IS
%token COMMA
%token LBRACKET
%token RBRACKET
%token TOKENERROR
%type <block> parameters
%type <val> value
%type <vec> values

%% /* Grammar rules and actions follow */

command:   parameters {
               KeyParser::setMap ($1);
           }
       ;

parameters: parameters COMMA NAME IS value {
               $$ = $1;
	       (*$$)[$3->getString()] = *$5;
	       delete $3;
	       delete $5;
	   }
       |   NAME IS value {
               $$ = new KeyValueMap;
	       (*$$)[$1->getString()] = *$3;
	       delete $1;
	       delete $3;
	   }
       ;

value:     LITERAL
	   {   $$ = $1; }
       |   LBRACKET values RBRACKET {
	       $$ = new KeyValue (*$2);
	       delete $2;
           }
       |   LBRACKET parameters RBRACKET {
	       $$ = new KeyValue (*$2);
	       delete $2;
           }
       |   LBRACKET IS RBRACKET {
	       /* Like in glish [=] is the syntax for an empty 'record' */
	       $$ = new KeyValue (KeyValueMap());
           }
       ;

values:    values COMMA LITERAL {
               $$ = $1;
	       $$->push_back (*$3);
	       delete $3;
	   }
       |   LITERAL {
               $$ = new vector<KeyValue>();
	       $$->push_back (*$1);
	       delete $1;
	   }
       |
           {   $$ = new vector<KeyValue>(); }
       ;

%%
