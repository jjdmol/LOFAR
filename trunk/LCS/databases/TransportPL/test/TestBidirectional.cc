//# Example.cc: Test program for bidirectional data transport between DHs
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Transport/TH_Mem.h>
#include <Transport/TH_File.h>
#include <TransportPL/TH_PL.h>
#include <DH_Example.h>

using namespace LOFAR;

bool testMem()
{
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // Assign an ID for each dataholder by hand for now
  // This will be done by the framework later on
  DH1.setID(1);
  DH2.setID(2);

  // connect DH1 to DH2 bidirectionally with non-blocking in-memory 
  // communication
  DH1.connectBidirectional(DH2, TH_Mem(), TH_Mem(), false);
    
  // initialize
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBuffer()[0] = makefcomplex(17,-3.5);
  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;
    
  // do the "forward" data transport
  DH1.write();
  DH2.read();
  
  cout << "After forward transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
  ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect" << endl;
    return false;
  }

  // now we change the data somewhat and do the "backwards" transport
  DH2.getBuffer()[0] += makefcomplex(1,1);
  DH2.setCounter(3);
  DH2.write();
  DH1.read();
  // 
  
  cout << "After backwards transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
  ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect after backwards transport" << endl;
    return false;
  }
  else {
    return true;
  }
}

bool testPL()
{
  TH_PL::useDatabase ("test");

  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // Assign an ID for each dataholder by hand for now
  // This will be done by the framework later on
  DH1.setID(1);
  DH2.setID(2);

  // connect DH1 to DH2 via a database
  DH1.connectBidirectional(DH2, TH_PL("TestBidirectional"),
			   TH_PL("TestBidirectional"));

  // initialize
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBuffer()[0] = makefcomplex(17,-3.5);
  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;
    
  // do the "forward" data transport
  DH1.write();
  DH2.read();
  
  cout << "After forward transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
  ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect" << endl;
    return false;
  }

  // now we change the data somewhat and do the "backwards" transport
  DH2.getBuffer()[0] += makefcomplex(1,1);
  DH2.setCounter(3);
  DH2.write();
  DH1.read();
  // 
  
  cout << "After backwards transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
  ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect after backwards transport" << endl;
    return false;
  }
  else {
    return true;
  }
}


int main(int argc, const char** argv)
{
  INIT_LOGGER("TestBidirectional.log_prop");

  cout << "Bidirectional transport test program" << endl;
  
  bool resultMem = true;
  cout << "Testing TH_Mem..." << endl;
  resultMem = testMem();
  
  bool resultPL = true;
  cout << "Testing TH_PL..." << endl;
  resultPL = testPL();

  // Bidirectional communication with TH_File is not possible, because TH_File 
  // is either read-only or write-only.

  // Check if all tests succeeded
  if (resultMem && resultPL) {
    return 0;
  } else {
    return 1;
  }
}
