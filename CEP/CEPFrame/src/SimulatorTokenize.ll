/*
//  SimulatorTokenize.ll: Scanner for simulation programs
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
//
//  $Log$
//  Revision 1.1.1.1  2003/02/21 11:14:36  schaaf
//  copy from BaseSim tag "CEPFRAME"
//
//  Revision 1.2  2002/06/10 09:48:31  diepen
//
//  %[BugId: 38]%
//  Added command dhfile to the parser
//
//  Revision 1.1  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.3  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
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
#include "ParamValue.h"
#include "ParamBlock.h"
#include "SimulatorParseClass.h"
#include "SimulatorParse.h"

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
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("define");
	    return DEFCOMMAND;
          }

{CHECK}   {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("check");
	    return OTHERCOMMAND;
          }

{PRERUN}  {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("prerun");
	    return OTHERCOMMAND;
          }

{RUN}     {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("run");
	    return OTHERCOMMAND;
          }

{STEP}    {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("step");
	    return OTHERCOMMAND;
          }

{DUMP}    {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("dump");
	    return OTHERCOMMAND;
          }

{DHFILE} {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("dhfile");
	    return OTHERCOMMAND;
          }

{POSTRUN} {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue ("postrun");
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
            LOFAR::SimulatorParse::position() += yyleng;
            double value;
	    sscanf (SimulatorTokenizetext, "%lf%*c", &value);
            lvalp->val = new LOFAR::ParamValue (complex<double> (0, value));
	    return LITERAL;
	  }
{COMPLEX} {
            LOFAR::SimulatorParse::position() += yyleng;
            float value;
	    sscanf (SimulatorTokenizetext, "%f%*c", &value);
            lvalp->val = new LOFAR::ParamValue (complex<float> (0, value));
	    return LITERAL;
	  }
{DOUBLE}  {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue (atof(SimulatorTokenizetext));
	    return LITERAL;
	  }
{FLOAT}   {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue (float(atof(SimulatorTokenizetext)));
	    return LITERAL;
	  }
{INT}     {
            LOFAR::SimulatorParse::position() += yyleng;
            int ival = atoi(SimulatorTokenizetext);
            double dval = atof(SimulatorTokenizetext);
            /* Handle integers exceeding integer precision as doubles */
            if (ival < dval-0.1  ||  ival > dval+0.1) {
                lvalp->val = new LOFAR::ParamValue (dval);
            } else {
                lvalp->val = new LOFAR::ParamValue (ival);
            }
            return LITERAL;
	  }
{TRUE}    {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue (true);
	    return LITERAL;
	  }
{FALSE}   {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue (false);
	    return LITERAL;
	  }

 /*
 Strings can have quotes which have to be removed.
 Names can have escape characters to be removed.
 */
{NAME}    {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue (SimulatorTokenizetext);
	    return NAME;
	  }
{ESCNAME} {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue
                         (LOFAR::SimulatorParse::removeEscapes (SimulatorTokenizetext));
	    return NAME;
	  }
{STRING}  {
            LOFAR::SimulatorParse::position() += yyleng;
            lvalp->val = new LOFAR::ParamValue
                         (LOFAR::SimulatorParse::removeQuotes (SimulatorTokenizetext));
	    return LITERAL;
	  }

"="       { return IS; }

","       { return COMMA; }

"["       { return LBRACKET; }

"]"       { return RBRACKET; }

 /* Whitespace is skipped */
{WHITE}   { LOFAR::SimulatorParse::position() += yyleng; }

 /* Newline updates counters */
{NEWLINE} { LOFAR::SimulatorParse::line()++; LOFAR::SimulatorParse::position() = 0; }

 /* An unterminated string is an error */
{USTRING} { throw "SimulatorTokenize: Unterminated string"; }

 /* Any other character is invalid */
.         { return TOKENERROR; }

%%
