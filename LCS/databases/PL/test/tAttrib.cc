//#  tAttrib.cc: program to test the makeAttrib method(s).
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

#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <PL/Attrib.h>
#include "PO_tAttrib.h"

using namespace LOFAR::PL;
using namespace dtl;
using namespace std;

int main(int argc, const char* argv[])
{

  Debug::initLevels(argc, argv);

  try {

    cout << "attrib<Z>(\"s\")       = " << attrib<Z>("s")       << endl;
    cout << "attrib<Y>(\"s\")       = " << attrib<Y>("s")       << endl;
    cout << "attrib<Y>(\"z.s\")     = " << attrib<Y>("z.s")     << endl;
    cout << "attrib<X>(\"s\")       = " << attrib<X>("s")       << endl;

    cout << "attrib<A>(\"s\")       = " << attrib<A>("s")       << endl;
    cout << "attrib<B>(\"s\")       = " << attrib<B>("s")       << endl;
    cout << "attrib<B>(\"x.s\")     = " << attrib<B>("x.s")     << endl;
    cout << "attrib<B>(\"A::s\")    = " << attrib<B>("A::s")    << endl;

    cout << "attrib<C>(\"s\")       = " << attrib<C>("s")       << endl;
    cout << "attrib<C>(\"y.s\")     = " << attrib<C>("y.s")     << endl;
    cout << "attrib<C>(\"y.z.s\")   = " << attrib<C>("y.z.s")   << endl;
    cout << "attrib<C>(\"B::s\")    = " << attrib<C>("B::s")    << endl;
    cout << "attrib<C>(\"B::x.s\")  = " << attrib<C>("B::x.s")  << endl;
    cout << "attrib<C>(\"B::A::s\") = " << attrib<C>("B::A::s") << endl;

  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
  }

  return 0;
}
