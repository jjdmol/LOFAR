/*
//  AppControllerTokenize.ll: Scanner for simulation programs
//
//  Copyright (C) 2000-2002
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
*/


%{
#include <Common/KeyValueMap.h>
#include "AppControllerParseClass.h"
#include "AppControllerParse.h"

#undef YY_DECL
#define YY_DECL int yylex (YYSTYPE* lvalp)

%}

DEFINE   [Dd][Ee][Ff][Ii][Nn][Ee]
CHECK    [Cc][Hh][Ee][Cc][Kk]
PRERUN   [Pp][Rr][Ee][Rr][Uu][Nn]
RUN      [Rr][Uu][Nn]
STEP     [Ss][Tt][Ee][Pp]
DUMP     [Dd][Uu][Mm][Pp]
DHFILE   [Dd][Hh][Ff][Ii][Ll][Ee]
POSTRUN  [Pp][Oo][Ss][Tt][Rr][Uu][Nn]
QUIT     [Qq][Uu][Ii][Tt]

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

QSTRING   \"[^\"\n]*\"
ASTRING   \'[^\'\n]*\'
UQSTRING   \"[^\"\n]*\n
UASTRING   \'[^\'\n]*\n
STRING    ({QSTRING}|{ASTRING})+
USTRING   ({UQSTRING}|{UASTRING})+
NAME      [A-Za-z_]([A-Za-z_0-9])*
ESCNAME   ([A-Za-z0-9._~$]|(\\.))+


%%
{DEFINE}  {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("define");
	    return DEFCOMMAND;
          }

{CHECK}   {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("check");
	    return OTHERCOMMAND;
          }

{PRERUN}  {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("prerun");
	    return OTHERCOMMAND;
          }

{RUN}     {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("run");
	    return OTHERCOMMAND;
          }

{STEP}    {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("step");
	    return OTHERCOMMAND;
          }

{DUMP}    {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("dump");
	    return OTHERCOMMAND;
          }

{DHFILE} {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("dhfile");
	    return OTHERCOMMAND;
          }

{POSTRUN} {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue ("postrun");
	    return OTHERCOMMAND;
          }

{QUIT}    {
            yyterminate();
          }

 /* quit on EOF */
<<EOF>>   {
            yyterminate();
          }

";"       { return ENDCOMM; }

 /* Literals */
{DCOMPLEX} {
            LOFAR::AppControllerParse::position() += yyleng;
            double value;
	    sscanf (AppControllerTokenizetext, "%lf%*c", &value);
            lvalp->val = new LOFAR::KeyValue (complex<double> (0, value));
	    return LITERAL;
	  }
{COMPLEX} {
            LOFAR::AppControllerParse::position() += yyleng;
            float value;
	    sscanf (AppControllerTokenizetext, "%f%*c", &value);
            lvalp->val = new LOFAR::KeyValue (complex<float> (0, value));
	    return LITERAL;
	  }
{DOUBLE}  {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue (atof(AppControllerTokenizetext));
	    return LITERAL;
	  }
{FLOAT}   {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue (float(atof(AppControllerTokenizetext)));
	    return LITERAL;
	  }
{INT}     {
            LOFAR::AppControllerParse::position() += yyleng;
            int ival = atoi(AppControllerTokenizetext);
            double dval = atof(AppControllerTokenizetext);
            /* Handle integers exceeding integer precision as doubles */
            if (ival < dval-0.1  ||  ival > dval+0.1) {
                lvalp->val = new LOFAR::KeyValue (dval);
            } else {
                lvalp->val = new LOFAR::KeyValue (ival);
            }
            return LITERAL;
	  }
{TRUE}    {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue (true);
	    return LITERAL;
	  }
{FALSE}   {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue (false);
	    return LITERAL;
	  }

 /*
 Strings can have quotes which have to be removed.
 Names can have escape characters to be removed.
 */
{NAME}    {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue (AppControllerTokenizetext);
	    return NAME;
	  }
{ESCNAME} {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue
                         (LOFAR::AppControllerParse::removeEscapes (AppControllerTokenizetext));
	    return NAME;
	  }
{STRING}  {
            LOFAR::AppControllerParse::position() += yyleng;
            lvalp->val = new LOFAR::KeyValue
                         (LOFAR::AppControllerParse::removeQuotes (AppControllerTokenizetext));
	    return LITERAL;
	  }

"="       { return IS; }

","       { return COMMA; }

"["       { return LBRACKET; }

"]"       { return RBRACKET; }

 /* Whitespace is skipped */
{WHITE}   { LOFAR::AppControllerParse::position() += yyleng; }

 /* Newline updates counters */
{NEWLINE} { LOFAR::AppControllerParse::line()++; LOFAR::AppControllerParse::position() = 0; }

 /* An unterminated string is an error */
{USTRING} { throw "AppControllerTokenize: Unterminated string"; }

 /* Any other character is invalid */
.         { return TOKENERROR; }

%%
