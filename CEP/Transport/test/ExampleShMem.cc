//# ExampleShMem.cc: Test program for class TH_ShMem
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

#include <iostream>

#include <Transport/TH_ShMem.h>
#include <DH_Example.h>

using namespace LOFAR;

// This test program is meant for testing ShMem in an MPI environment.
// However, if there is no MPI, TH_ShMem is the same as TH_Mem.
// In that case the test program should run fine as well.

int main(int argc, const char* argv[])
{
  try {  
    cout << "Transport ExampleShMem test program" << endl;

    TH_ShMem::init (argc, argv);
    
    DH_Example DH1("dh1", 1);
    DH1.runOnNode (0);
    DH_Example DH2("dh2", 1);
    DH2.runOnNode (1);
    
    // Assign an ID for each transporter by hand for now
    // This will be done by the framework later on
    DH1.setID(1);
    DH2.setID(2);

    DH1.setBlocking(false);
    DH2.setBlocking(false);

    // connect DH1 to DH2
    DH1.connectTo(DH2, TH_ShMem());
    
    // initialize the DataHolders
    DH1.init();
    DH2.init();
    
    // fill the DataHolders with some initial data
    DH1.getBuffer()[0] = fcomplex(17,-3.5);
    DH2.getBuffer()[0] = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);

    int rank = TH_ShMem::getCurrentRank();
    cout << rank << endl;
    int sts=0;
    if (rank <= 0 ) {
      cout << "Before transport : " 
	   << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
	   << " -- " 
	   << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
	   << endl;
      DH1.write();
      cout << "write done" << endl;
    }
    if (rank == 1  ||  rank < 0) {
      DH2.read();
      cout << "read done" << endl;
      cout << "After transport  : " 
	   << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
	   << " -- " 
	   << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
	   << endl;

      if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
	  ||  DH1.getCounter() != DH2.getCounter()) {
	cout << "Data in receiving DataHolder is incorrect" << endl;
	sts = 1;
      }
    }
    TH_ShMem::finalize();
    return sts;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
