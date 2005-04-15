//#  dComplex.cc: demo program showing the use the PL with complex numbers.
//#
//#  Copyright (C) 2002-2003
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

// \file dComplex.cc
// Demo program showing the use the PL with complex numbers.

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "PO_Complex.h"
#include <PL/PersistenceBroker.h>
#include <PL/TPersistentObject.h>
#include <iostream>

using namespace LOFAR::PL;
using namespace std;

int main()
{
  try {
    PersistenceBroker b;
    b.connect("test", "postgres");
    while (cin) {
      Complex c;
      cout << "Enter a complex number (Ctrl-D to skip) : ";
      cin >> c;
      if (cin) b.save(TPersistentObject<Complex>(c));
    }
    typedef Collection< TPersistentObject<Complex> > CPOCmplx;
    CPOCmplx cc = b.retrieve<Complex>(QueryObject());
    cout << endl << "Retrieved " << cc.size() << " objects." << endl;
    CPOCmplx::const_iterator cit;
    for(cit = cc.begin(); cit != cc.end(); ++cit) {
      cout << cit->data() << endl;
    }
  } catch (LOFAR::Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
