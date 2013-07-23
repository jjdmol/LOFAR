/*
//# KeyTokenize.ll: Scanner for key=value line
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

/* yy_unput is not used, so let flex not generate it, otherwise picky
   compilers will issue warnings. */
%option nounput

%{
#include <Blob/KeyValue.h>
#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include "KeyParse.h"           // output of bison
#include <Common/lofar_iostream.h>
using namespace LOFAR;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result=KeyParser::input(buf,max_size)

#undef YY_DECL
#define YY_DECL int yylex (YYSTYPE* lvalp)
%}

WHITE     [ \t\n]*
DIGIT     [0-9]
INT       [+-]?{DIGIT}+
DEXP      [Ee][+-]?{INT}
DOUBLE    [+-]?({INT}{DEXP}|{INT}"."{DIGIT}*({DEXP})?|{DIGIT}*"."{INT}({DEXP})?)
DBINT     {DOUBLE}|{INT}
FLOAT     {DBINT}[fF]
COMPLEX   {FLOAT}"+"{FLOAT}[ij]
DCOMPLEX  {DBINT}"+"{DBINT}[ij]
NCOMPLEX  {FLOAT}"-"{FLOAT}[ij]
NDCOMPLEX {DBINT}"-"{DBINT}[ij]
IMAG      {FLOAT}[ij]
DIMAG     {DBINT}[ij]
TRUE      T
FALSE     F
DMS       ({INT})?"."({INT})?"."({INT}|{FLOAT}|{DOUBLE})?
HMS       ({INT})?":"({INT})?":"({INT}|{FLOAT}|{DOUBLE})?

QSTRING   \"[^\"\n]*\"
ASTRING   \'[^\'\n]*\'
UQSTRING   \"[^\"\n]*\n
UASTRING   \'[^\'\n]*\n
STRING    ({QSTRING}|{ASTRING})+
USTRING   ({UQSTRING}|{UASTRING})+
NAME      [A-Za-z_]([A-Za-z_0-9])*
ESCNAME   ([A-Za-z0-9._~$]|(\\.))+
COMMENT   "#".*"\n"


%%

 /* quit on EOF */
<<EOF>>   {
            yyterminate();
          }

 /* Literals */
{DMS}     {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (KeyTokenizetext, KeyValue::DMS);
	    return LITERAL;
	  }
{HMS}     {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (KeyTokenizetext, KeyValue::HMS);
	    return LITERAL;
	  }
{COMPLEX} {
            KeyParser::position() += yyleng;
            float valr,vali;
	    sscanf(KeyTokenizetext, "%f%*c+%f%*c", &valr, &vali);
            lvalp->val = new KeyValue (makefcomplex(valr, vali));
	    return LITERAL;
	  }
{DCOMPLEX} {
            KeyParser::position() += yyleng;
            double valr,vali;
	    sscanf(KeyTokenizetext, "%lf+%lf", &valr, &vali);
            lvalp->val = new KeyValue (makedcomplex(valr, vali));
	    return LITERAL;
	  }
{NCOMPLEX} {
            KeyParser::position() += yyleng;
            float valr,vali;
	    sscanf(KeyTokenizetext, "%f%*c-%f%*c", &valr, &vali);
            lvalp->val = new KeyValue (makefcomplex(valr, -vali));
	    return LITERAL;
	  }
{NDCOMPLEX} {
            KeyParser::position() += yyleng;
            double valr,vali;
	    sscanf(KeyTokenizetext, "%lf-%lf", &valr, &vali);
            lvalp->val = new KeyValue (makedcomplex(valr, -vali));
	    return LITERAL;
	  }
{IMAG} {
            KeyParser::position() += yyleng;
            float vali;
	    sscanf(KeyTokenizetext, "%f%*c", &vali);
            lvalp->val = new KeyValue (makefcomplex(0., vali));
	    return LITERAL;
	  }
{DIMAG} {
            KeyParser::position() += yyleng;
            double vali;
	    sscanf(KeyTokenizetext, "%lf", &vali);
            lvalp->val = new KeyValue (makedcomplex(0., vali));
	    return LITERAL;
	  }
{FLOAT}   {
            KeyParser::position() += yyleng;
            float val;
	    sscanf(KeyTokenizetext, "%f%*c", &val);
            lvalp->val = new KeyValue (val);
	    return LITERAL;
	  }
{DOUBLE}  {
            KeyParser::position() += yyleng;
            double val;
	    sscanf(KeyTokenizetext, "%lf", &val);
            lvalp->val = new KeyValue (val);
	    return LITERAL;
	  }
{INT}     {
            KeyParser::position() += yyleng;
            int ival = atoi(KeyTokenizetext);
            double dval = atof(KeyTokenizetext);
            /* Handle integers exceeding integer precision as doubles */
            if (ival < dval-0.1  ||  ival > dval+0.1) {
                lvalp->val = new KeyValue (dval);
            } else {
                lvalp->val = new KeyValue (ival);
            }
            return LITERAL;
	  }
{TRUE}    {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (true);
	    return LITERAL;
	  }
{FALSE}   {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (false);
	    return LITERAL;
	  }

 /*
 Strings can have quotes which have to be removed.
 Names can have escape characters to be removed.
 */
{NAME}    {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (KeyTokenizetext);
	    return NAME;
	  }
{ESCNAME} {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue
                         (KeyParser::removeEscapes (KeyTokenizetext));
	    return NAME;
	  }
{STRING}  {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue
                         (KeyParser::removeQuotes (KeyTokenizetext));
	    return LITERAL;
	  }

"="       { KeyParser::position() += yyleng; return IS; }

","       { KeyParser::position() += yyleng; return COMMA; }

"["       { KeyParser::position() += yyleng; return LBRACKET; }

"]"       { KeyParser::position() += yyleng; return RBRACKET; }

 /* Whitespace is skipped */
{WHITE}   { KeyParser::position() += yyleng; }

 /* Comments are skipped */
{COMMENT} { KeyParser::position() += yyleng; }

 /* An unterminated string is an error */
{USTRING} { throw "KeyTokenize: Unterminated string"; }

 /* Any other character is invalid */
.         { return TOKENERROR; }

%%
