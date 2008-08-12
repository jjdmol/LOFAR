//#  tObjectId.cc: Test program for the ObjectId class
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
#include <PL/ObjectId.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <sys/time.h>

using namespace LOFAR;
using namespace LOFAR::PL;

int main()
{
  ObjectId oid1;
  ObjectId oid2;
  cout << hex << setfill('0');
  oid1.get();
  cout << endl << "oid1.get() = >>>0x" << oid1.get() << "<<<";
  cout << endl << "oid2.get() = >>>0x" << oid2.get() << "<<<";
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  cout << endl << "Assiging oid1 to oid2...";
  oid2.set(oid1.get());
  cout << endl << "oid2.get() = >>>0x" << oid2.get() << "<<<";
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  cout << endl << "Making oid1 and oid2 equal to global null object-id ...";
  oid1.set(0);
  oid2.set(0);
  cout << endl << "oid1.get() = 0x" << oid1.get();
  cout << endl << "oid2.get() = 0x" << oid2.get();
  cout << endl << "(oid1 == oid2) = " << (oid1 == oid2 ? "true" : "false") 
       << endl;

  timeval tv_start, tv_end;
  double sec;
  cout << endl << "Generating 1.000.000 uninitialized Object-IDs ...";
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId(0).get();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << endl << "Elapsed time: >>>" << sec << "<<< seconds." << endl;

  cout << endl << "Generating 1.000.000 initialized Object-IDs ...";
  gettimeofday(&tv_start,0);
  for (int i=0; i<1000000; i++) {
    ObjectId().get();
  }
  gettimeofday(&tv_end,0);
  sec = (tv_end.tv_sec - tv_start.tv_sec) + 
        (tv_end.tv_usec - tv_start.tv_usec) * 1e-6;
  cout << endl << "Elapsed time: >>>" << sec << "<<< seconds." << endl;

  return 0;
}
