//# tExamplePL.cc: Test program for classes DH_PL and TH_PL
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

#include <Transport/Transporter.h>
#include <Transport/TH_PL.h>
#include <DH_Example.h>
#include <iostream>


using namespace LOFAR;

int main()
{
  try {
    cout << "Transport ExamplePL test program" << endl;

    TH_PL::useDatabase ("test");

    DH_Example DH1("dh1", 1);
    DH_Example DH2("dh2", 1);
    Transporter& TR1 = DH1.getTransporter();
    Transporter& TR2 = DH2.getTransporter();
    
    // Assign an ID for each transporter by hand for now
    // This will be done by the framework later on
    TR1.setItsID(1);
    TR2.setItsID(2);
 
    // connect DH1 to DH2
    TH_PL TH1("ExamplePL");
    TR1.connectTo(TR2, TH1);
    
    // initialize the TransportHolders and DataHolders
    TR1.init();
    TR2.init();
    
    // fill the DataHolders with some initial data
    DH1.getBuffer()[0] = fcomplex(17,-3.5);
    DH2.getBuffer()[0] = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    
    cout << "Before transport : " 
	 << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
	 << " -- " 
	 << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
	 << endl;
    
    // do the data transport
    DH1.write();
    DH2.read();
    // note that transport is bi-directional.
    // so this will also work:
    //   DH2.write();
    //   DH1.read();
    // 
  
    cout << "After transport  : " 
	 << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
	 << " -- " 
	 << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
	 << endl;

    if (DH1.getBuffer()[0] == DH2.getBuffer()[0]
    &&  DH1.getCounter() == DH2.getCounter()) {
      return 0;
    }
    cout << "Data in receiving DataHolder is incorrect" << endl;
    return 1;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
