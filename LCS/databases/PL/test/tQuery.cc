//#  tQuery.cc: Small test program for the QueryObject class.
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

#include <PL/Query.h>
#include <PL/Query/Expr.h>
#include <Common/Debug.h>
#include <string>
#include <iostream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::PL;

int main(int argc, const char* argv[])
{
  Debug::initLevels(argc, argv);

  string      s("string");
  const char* c("const char *");
  Query::Expr e("Query::Expr");

  cout << "QueryObject(s).getSql() = " << QueryObject(s).getSql() << endl;
  cout << "QueryObject(c).getSql() = " << QueryObject(c).getSql() << endl;
  cout << "QueryObject(e).getSql() = " << QueryObject(e).getSql() << endl;

  cout << "QueryObject(\"\").getSql() = " 
       << QueryObject("").getSql() << endl;
  cout << "QueryObject(Query::Expr()) = " 
       << QueryObject(Query::Expr()).getSql() << endl;
}
