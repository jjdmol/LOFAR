/*
//  SimulatorParse.y: Scanner for simulation programs
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.1.1.1  2003/02/21 11:14:36  schaaf
//  copy from BaseSim tag "CEPFRAME"
//
//  Revision 1.2  2002/05/08 14:15:57  wierenga
//  Added correct include paths
//
//  Revision 1.1  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.1  2001/03/01 13:17:03  gvd
//  New parser source files
//
//
/////////////////////////////////////////////////////////////////////////////
*/

%{
#include "CEPFrame/SimulatorParseClass.h"
#include "CEPFrame/ParamBlock.h"
%}

%pure_parser                /* make parser re-entrant */

%union {
ParamValue* val;
ParamBlock* block;
vector<ParamValue>* vec;
}

%{
int SimulatorTokenizelex (YYSTYPE*);
#define SimulatorParselex SimulatorTokenizelex
%}


%token <val> LITERAL
%token <val> NAME
%token <val> DEFCOMMAND
%token <val> OTHERCOMMAND
%token ENDCOMM
%token IS
%token COMMA
%token LBRACKET
%token RBRACKET
%token TOKENERROR
%type <block> parameters
%type <val> value
%type <vec> values

%% /* Grammar rules and actions follow */

command:   defcom
       |   defcom restcom
       |   command defcom
       |   command defcom restcom
       ;

defcom:    DEFCOMMAND parameters ENDCOMM {
               SimulatorParse::execute (*$1, *$2);
	       delete $1;
	       delete $2;
           }
       |   DEFCOMMAND ENDCOMM {
               SimulatorParse::execute (*$1, ParamBlock());
	       delete $1;
           }
       ;

restcom:   othercom
       |   restcom othercom
       ;

othercom:  OTHERCOMMAND ENDCOMM {
               SimulatorParse::execute (*$1);
	       delete $1;
           }
       |   OTHERCOMMAND value ENDCOMM {
               SimulatorParse::execute (*$1, *$2);
	       delete $1;
	       delete $2;
           }
       |   OTHERCOMMAND parameters ENDCOMM {
               SimulatorParse::execute (*$1, *$2);
	       delete $1;
	       delete $2;
           }
       ;

parameters: parameters COMMA NAME IS value {
               $$ = $1;
	       (*$$)[$3->getString()] = *$5;
	       delete $3;
	       delete $5;
	   }
       |   NAME IS value {
               $$ = new ParamBlock;
	       (*$$)[$1->getString()] = *$3;
	       delete $1;
	       delete $3;
	   }
       ;

value:     LITERAL
	   {   $$ = $1; }
       |   LBRACKET values RBRACKET {
	       $$ = new ParamValue (*$2);
	       delete $2;
           }
       |   LBRACKET parameters RBRACKET {
	       $$ = new ParamValue (*$2);
	       delete $2;
           }
       |   LBRACKET IS RBRACKET {
	       /* Like in glish [=] is the syntax for an empty 'record' */
	       $$ = new ParamValue (ParamBlock());
           }
       ;

values:    values COMMA LITERAL {
               $$ = $1;
	       $$->push_back (*$3);
	       delete $3;
	   }
       |   LITERAL {
               $$ = new vector<ParamValue>();
	       $$->push_back (*$1);
	       delete $1;
	   }
       |
           {   $$ = new vector<ParamValue>(); }
       ;

%%
