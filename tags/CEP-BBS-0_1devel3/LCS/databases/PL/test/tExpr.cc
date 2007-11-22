//#  tExpr.cc: Test program for the query expression node classes.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <PL/Query/Expr.h>
#include <PL/Query/ColumnExprNode.h>
#include <PL/Collection.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_string.h>
#include <stdexcept>

#define WRITE(os, strm, width) \
  try { \
    ostringstream oss; \
    oss << strm; \
    string s(oss.str()); \
    os << setw(width) << left << s; \
    if (0 < width && width < s.size()) os << endl << string(width, ' '); \
  } catch (std::exception& e) { \
    os << e.what(); \
  }

#define WRITELN(os, strm, width) \
  WRITE(os, strm, width); \
  os << endl

#define EXPR(arg) \
  Expr(arg)

#define EXPR_STR(arg) \
  "Expr(" << #arg << ")"

#define CONST_EXPR(arg) \
  WRITE(cout, EXPR_STR(arg), 50); \
  WRITE(cout, " : ", 0); \
  WRITELN(cout, EXPR(arg), 0)

#define UNARY(oper, arg) \
  oper EXPR(arg)

#define UNARY_STR(oper, arg) \
  #oper << EXPR_STR(arg)
  
#define UNARY_EXPR(oper, arg) \
  WRITE(cout, UNARY_STR(oper, arg), 50); \
  WRITE(cout, " : ", 0); \
  WRITELN(cout, UNARY(oper, arg), 0)

#define BINARY(lhs, oper, rhs) \
  EXPR(lhs) oper EXPR(rhs)

#define BINARY_STR(lhs, oper, rhs) \
  EXPR_STR(lhs) << " " << #oper << " " << EXPR_STR(rhs)

#define BINARY_EXPR(lhs, oper, rhs) \
  WRITE(cout, BINARY_STR(lhs, oper, rhs), 50); \
  WRITE(cout, " : ", 0); \
  WRITELN(cout, (BINARY(lhs, oper, rhs)), 0)

#define SQL_UNARY(expr, oper, arg) \
  EXPR(expr).oper(arg)

#define SQL_UNARY_STR(expr, oper, arg) \
  EXPR_STR(expr) << "." << #oper << "(" << #arg << ")"

#define SQL_UNARY_EXPR(expr, oper, arg) \
  WRITE(cout, SQL_UNARY_STR(expr, oper, arg), 50); \
  WRITE(cout, " : ", 0); \
  WRITELN(cout, SQL_UNARY(expr, oper, arg), 0)

#define SQL_BINARY(expr, oper, lhs, rhs) \
  EXPR(expr).oper(EXPR(lhs),EXPR(rhs))

#define SQL_BINARY_STR(expr, oper, lhs, rhs) \
  EXPR_STR(expr) << "." << #oper \
          << "(" << EXPR_STR(lhs) << "," << EXPR_STR(rhs) << ")"

#define SQL_BINARY_EXPR(expr, oper, lhs, rhs) \
  WRITE(cout, SQL_BINARY_STR(expr, oper, lhs, rhs), 50); \
  WRITE(cout, " : ", 0); \
  WRITELN(cout, SQL_BINARY(expr, oper, lhs, rhs), 0)

using namespace LOFAR;
using namespace LOFAR::PL;
using namespace LOFAR::PL::Query;

int main()
{
  // Constant expressions
  {
    cout << endl << "=== Constant expressions ===" << endl;
    CONST_EXPR(1);
    CONST_EXPR(3.14);
    CONST_EXPR("Where's Johnny?!");
  }

  // Arithmetic expressions
  {
    cout << endl << "=== Arithmetic expressions ===" << endl;
    UNARY_EXPR(+,);
    UNARY_EXPR(-,);
    UNARY_EXPR(+,1);
    UNARY_EXPR(-,2.0);
    BINARY_EXPR(,+,);
    BINARY_EXPR(0,-,);
    BINARY_EXPR(,*,0);
    BINARY_EXPR(0,/,0);
    BINARY_EXPR(1,+,-2);
    BINARY_EXPR(2.0,-,3);
    BINARY_EXPR(-3,*,-4.2);
    BINARY_EXPR(-4.1,/,2.7);
    BINARY_EXPR(BINARY(3,+,4),*,BINARY(5,-,6));
  }

  // Comparison expressions
  {
    cout << endl << "=== Comparison expressions ===" << endl;
    BINARY_EXPR(,<,);
    BINARY_EXPR(0,>,);
    BINARY_EXPR(0,==,0);
    BINARY_EXPR(-1,!=,1);
    BINARY_EXPR(2.0,>=,3);
    BINARY_EXPR(4,>,2.7);
    BINARY_EXPR(-1.5,<=,3);
    BINARY_EXPR(3,<,-2.7);
  }

  // Logical expressions
  {
    cout << endl << "=== Logical expressions ===" << endl;
    UNARY_EXPR(!,0);
    BINARY_EXPR(,||,);
    BINARY_EXPR(,||,0);
    BINARY_EXPR(0,&&,);
    BINARY_EXPR(0,&&,0);
    BINARY_EXPR(BINARY(,==,),&&,BINARY(0,!=,0));
    BINARY_EXPR(BINARY(,<,0),||,BINARY(0,>,));
    BINARY_EXPR(BINARY(0,<=,1),&&,BINARY(1,>=,0));
    BINARY_EXPR(UNARY(!,1),||,UNARY(!,-1));
    BINARY_EXPR("hello",!=,"world");
  }

  // SQL-like expressions
  {
    cout << endl << "=== SQL BETWEEN expressions ===" << endl; 
    SQL_BINARY_EXPR(,between,,);
    SQL_BINARY_EXPR(,between,0,);
    SQL_BINARY_EXPR(,between,-1,1);
    SQL_BINARY_EXPR(0,between,,);
    SQL_BINARY_EXPR(0,between,-1,);
    SQL_BINARY_EXPR(0,between,UNARY(-,1),1);
    SQL_BINARY_EXPR(BINARY(5,-,2),between,2,4);
    SQL_BINARY_EXPR(BINARY(2,-,5),notBetween,2,4);

    cout << endl << "=== SQL IN expressions ===" << endl;
    Collection<Expr> c;
    SQL_UNARY_EXPR(1,in,c);
    c.add(2);
    SQL_UNARY_EXPR(BINARY(4,+,3),in,c);
    c.add(3);
    c.add(4);
    SQL_UNARY_EXPR(BINARY(3,*,UNARY(-,4)),notIn,c);

    cout << endl << "=== SQL LIKE expressions ===" << endl;
    SQL_UNARY_EXPR("Hello_World",like,"Hello_*");         // true
    SQL_UNARY_EXPR("Hello_World",like,"*_World");         // true
    SQL_UNARY_EXPR("Hello_World",like,"Hello_?");         // false
    SQL_UNARY_EXPR("Hello_World",like,"?_World");         // false
    SQL_UNARY_EXPR("Hello_World",like,"Hello?World");     // true
    cout << endl;
    SQL_UNARY_EXPR("Hello%World",notLike,"Hello%*");      // false
    SQL_UNARY_EXPR("Hello%World",notLike,"*%World");      // false
    SQL_UNARY_EXPR("Hello%World",notLike,"Hello%?");      // true
    SQL_UNARY_EXPR("Hello%World",notLike,"?%World");      // true
    SQL_UNARY_EXPR("Hello%World",notLike,"Hello?World");  // false
    cout << endl;
    SQL_UNARY_EXPR("Hello*World",like,"Hello\\**");       // true
    SQL_UNARY_EXPR("Hello*World",like,"*\\*World");       // true
    SQL_UNARY_EXPR("Hello*World",like,"Hello\\*?");       // false
    SQL_UNARY_EXPR("Hello*World",like,"?\\*World");       // false
    SQL_UNARY_EXPR("Hello*World",like,"Hello?World");     // true
    cout << endl;
    SQL_UNARY_EXPR("Hello?World",notLike,"Hello\\?*");    // false
    SQL_UNARY_EXPR("Hello?World",notLike,"*\\?World");    // false
    SQL_UNARY_EXPR("Hello?World",notLike,"Hello\\??");    // true
    SQL_UNARY_EXPR("Hello?World",notLike,"?\\?World");    // true
    SQL_UNARY_EXPR("Hello?World",notLike,"Hello?World");  // false
    cout << endl;
    SQL_UNARY_EXPR("Hello\\\\World",like,"Hello\\\\*");   // true
    SQL_UNARY_EXPR("Hello\\\\World",like,"*\\\\World");   // true
    SQL_UNARY_EXPR("Hello\\\\World",like,"Hello\\\\?");   // false
    SQL_UNARY_EXPR("Hello\\\\World",like,"?\\\\World");   // false
    SQL_UNARY_EXPR("Hello\\\\World",like,"Hello?World");  // true
    cout << endl;
    SQL_UNARY_EXPR("Hello'World",notLike,"Hello'*");      // false
    SQL_UNARY_EXPR("Hello'World",notLike,"*'World");      // false
    SQL_UNARY_EXPR("Hello'World",notLike,"Hello'?");      // true
    SQL_UNARY_EXPR("Hello'World",notLike,"?'World");      // true
    SQL_UNARY_EXPR("Hello'World",notLike,"Hello?World");  // false
  }

  {
    cout << endl << "=== Invalid composite expressions ===" << endl;
    BINARY_EXPR(SQL_BINARY(0,between,-1,2),+,3);
    SQL_UNARY_EXPR(BINARY("Hello",+,"World"),like,"Hello*");
  }

  return 0;
}
