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

%{
#include <KeyValue.h>
#include <KeyValueMap.h>
#include <KeyParser.h>
#include <KeyParse.h>     // output of bison
using namespace LOFAR;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result=KeyParser::input(buf,max_size)

#undef YY_DECL
#define YY_DECL int yylex (YYSTYPE* lvalp)
%}

WHITE     [ \t]*
NEWLINE   [\n]
DIGIT     [0-9]
INT       [+-]?{DIGIT}+
FEXP      [Ee][+-]?{INT}
DEXP      [Dd][+-]?{INT}
FLOAT     [+-]?{INT}{FEXP}|{INT}"."{DIGIT}*({FEXP})?|{DIGIT}*"."{INT}({FEXP})?
DOUBLE    [+-]?{INT}{DEXP}|{INT}"."{DIGIT}*({DEXP})?|{DIGIT}*"."{INT}({DEXP})?
FLINT     {FLOAT}|{INT}
DBINT     {DOUBLE}|{INT}
COMPLEX   {FLINT}+{FLINT}"i"
DCOMPLEX  {DBINT}+{DBINT}"i"
IMAG      {FLINT}"i"
DIMAG     {DBINT}"i"
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
{DCOMPLEX} {
            KeyParser::position() += yyleng;
            double value;
	    sscanf (KeyTokenizetext, "%lf%*c", &value);
            lvalp->val = new KeyValue (complex<double> (0, value));
	    return LITERAL;
	  }
{COMPLEX} {
            KeyParser::position() += yyleng;
            float value;
	    sscanf (KeyTokenizetext, "%f%*c", &value);
            lvalp->val = new KeyValue (complex<float> (0, value));
	    return LITERAL;
	  }
{DOUBLE}  {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (atof(KeyTokenizetext));
	    return LITERAL;
	  }
{FLOAT}   {
            KeyParser::position() += yyleng;
            lvalp->val = new KeyValue (float(atof(KeyTokenizetext)));
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

"="       { return IS; }

","       { return COMMA; }

"["       { return LBRACKET; }

"]"       { return RBRACKET; }

 /* Whitespace is skipped */
{WHITE}   { KeyParser::position() += yyleng; }

 /* An unterminated string is an error */
{USTRING} { throw "KeyTokenize: Unterminated string"; }

 /* Any other character is invalid */
.         { return TOKENERROR; }

%%
