//# Example.cc: Test program for bidirectional data transport between DHs
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <iostream>

#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <DH_Example.h>

using namespace LOFAR;

bool testMem()
{
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // connect DH1 to DH2 bidirectionally with non-blocking in-memory 
  // communication
  // Use a different TH_Mem in each connection! This is because 
  // (in non-blocking mode) during the first read/write call the 
  // source address is set in TH_Mem. This source is used in all 
  // further communication.
  TH_Mem memTH1;    
  TH_Mem memTH2;
  // A connection for data flowing from DH1 to DH2.
  Connection conn1("connection1", &DH1, &DH2, &memTH1, false);
  // And a connection for data flowing from DH2 to DH1.
  Connection conn2("connection2", &DH2, &DH1, &memTH2, false);
    
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
  ASSERT(conn1.write() == Connection::Finished);
  ASSERT(conn1.read() == Connection::Finished);
  
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
  ASSERT(conn2.write() == Connection::Finished);
  ASSERT(conn2.read() == Connection::Finished);
   
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


int main()
{
  INIT_LOGGER("TestBidirectional.log_prop");

  cout << "Bidirectional transport test program" << endl;
  
  bool resultMem = true;
  cout << "Testing TH_Mem..." << endl;
  resultMem = testMem();
  
  // Bidirectional communication with TH_File is not possible, because TH_File 
  // is either read-only or write-only.

  // Check if all tests succeeded
  if (resultMem) {
    return 0;
  } else {
    return 1;
  }
}
