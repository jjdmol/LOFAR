//# tExamplePL3.cc: Test program for classes DH_PL and TH_PL
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

#include <TransportPL/TH_PL.h>
#include <DH_Example2.h>
#include <Common/LofarLogger.h>
#include <iostream>


using namespace LOFAR;

int main()
{
  try {
    INIT_LOGGER("ExamplePL3.log_prop");

    cout << "Transport ExamplePL3 test program" << endl;

    TH_PL::useDatabase ("test");

    DH_Example2 DH1("dh1", 1);
    DH_Example2 DH2("dh2", 1);
    
    // Assign an ID for each dataholder by hand for now
    // This will be done by the framework later on
    DH1.setID(1);
    DH2.setID(2);
 
    // connect DH1 to DH2
    TH_PL TH1("ExamplePL2");
    DH1.connectTo(DH2, TH1);
    
    // initialize the DataHolders
    DH1.init();
    DH2.init();
    
    // fill the DataHolders with some initial data
    DH1.getBuffer()[0] = fcomplex(17,-3.5);
    DH2.getBuffer()[0] = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    
    // do the data transport
    DH1.write();
    ASSERT (DH2.queryDB ("counter=2 order by counter") == 1);
    ASSERT (DH2.getBuffer()[0] == fcomplex(17,-3.5)
	    &&  DH2.getCounter() == 2);

    // do the data transport again with different values.
    DH1.getBuffer()[0] = fcomplex(117,-13.15);
    DH1.setCounter(10);
    DH1.write();
    DH1.getBuffer()[0] = fcomplex(200,114);
    DH1.setCounter(21);
    DH1.write();

    ASSERT (DH2.queryDB ("counter>2 order by counter") == 2);
    ASSERT (DH2.getBuffer()[0] == fcomplex(117,-13.15)
	    &&  DH2.getCounter() == 10);
    ASSERT (DH2.queryDB ("counter<4") == 1);
    ASSERT (DH2.getBuffer()[0] == fcomplex(17,-3.5)
	    &&  DH2.getCounter() == 2);
    ASSERT (DH2.queryDB ("counter>2 order by counter desc") == 2);
    ASSERT (DH2.getBuffer()[0] == fcomplex(200,114)
	    &&  DH2.getCounter() == 21);
    DH2.setCounter(22);
    DH2.updateDB();
    ASSERT (DH2.queryDB ("counter>2 order by counter desc") == 2);
    ASSERT (DH2.getBuffer()[0] == fcomplex(200,114)
	    &&  DH2.getCounter() == 22);
    return 0;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
