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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include "PO_tAttrib.h"
#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <PL/Attrib.h>

using namespace LOFAR;
using namespace LOFAR::PL;

//
//  This test program tests the conversion of a C++-like attribute
//  specification into an SQL-like WHERE clause. You should understand
//  the concept of "ownership" as it is used by the TPersistentObject
//  class. The generated attrib<>() method will convert inheritance and
//  composite relationships into ownership constraints.
//
//  For example:
//  \code
//      cout << attrib<D>("c.A::s") << endl;
//  \endcode
//  should produce the following output:
//  \verbatim
//      A.S AND ((C.OBJID=A.OWNER) AND (D.OBJID=C.OWNER))
//  \endverbatim
//
int main(int argc, const char* argv[])
{
  // Initialize the logger
  INIT_LOGGER(argv[0]);
  try {
    cout << endl;
    cout << "attrib<A>(\"s\")         == \"A\" --> " 
         << (attrib<A>("s") == "A") << endl;
    cout << "attrib<C>(\"A::s\")      == \"A\" --> " 
         << (attrib<C>("A::s") == "A") << endl;
    cout << "attrib<D>(\"c.A::s\")    == \"A\" --> " 
         << (attrib<D>("c.A::s") == "A") << endl;
    cout << "attrib<G>(\"D::c.A::s\") == \"A\" --> " 
         << (attrib<G>("D::c.A::s") == "A") << endl;

    cout << endl;
    cout << "attrib<B>(\"s\")         == \"B\" --> "
         << (attrib<B>("s") == "B") << endl;
    cout << "attrib<D>(\"B::s\")      == \"B\" --> " 
         << (attrib<D>("B::s") == "B") << endl;
    cout << "attrib<G>(\"D::B::s\")   == \"B\" --> "
         << (attrib<G>("D::B::s") == "B") << endl;

    cout << endl;
    cout << "attrib<C>(\"s\")         == \"C\" --> "
         << (attrib<C>("s") == "C") << endl;
    cout << "attrib<D>(\"c.s\")       == \"C\" --> "
         << (attrib<D>("c.s") == "C") << endl;
    cout << "attrib<G>(\"D::c.s\")    == \"C\" --> "
         << (attrib<G>("D::c.s") == "C") << endl;

    cout << endl;
    cout << "attrib<D>(\"s\")         == \"D\" --> " 
         << (attrib<D>("s") == "D") << endl;
    cout << "attrib<G>(\"D::s\")      == \"D\" --> "
         << (attrib<G>("D::s") == "D") << endl;

    cout << endl;
    cout << "attrib<E>(\"s\")         == \"E\" --> "
         << (attrib<E>("s") == "E") << endl;
    cout << "attrib<F>(\"e.s\")       == \"E\" --> " 
         << (attrib<F>("e.s") == "E") << endl;
    cout << "attrib<G>(\"f.e.s\")     == \"E\" --> " 
         << (attrib<G>("f.e.s") == "E") << endl;

    cout << endl;
    cout << "attrib<F>(\"s\")         == \"F\" --> "
         << (attrib<F>("s") == "F") << endl;
    cout << "attrib<G>(\"f.s\")       == \"F\" --> "
         << (attrib<G>("f.s") == "F") << endl;

    cout << endl;
    cout << "attrib<G>(\"s\")         == \"G\" --> "
         << (attrib<G>("s") == "G") << endl;

    cout << endl;
  }
  catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
