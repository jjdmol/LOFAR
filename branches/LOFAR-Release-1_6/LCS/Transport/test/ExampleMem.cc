//# ExampleMem.cc: Test program for basic Transport classes
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

#include <DH_Example.h>
#include <DH_ExampleExtra.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <iostream>

using namespace LOFAR;

bool test1()
{
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // connect DH1 to DH2 with non-blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection1", &DH1, &DH2, &memTH, false);
    
  // initialize
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBufferElement(0) = makefcomplex(17,-3.5);
  DH2.getBufferElement(0) = makefcomplex(0,0);
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);
  
  cout << "After transport  : " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
  &&  DH1.getCounter() == DH2.getCounter()) {
    return true;
  }
  cout << "Data in receiving DataHolder is incorrect" << endl;
  return false;
}

bool test2()
{
  DH_ExampleExtra DH1("dh1", 1);
  DH_ExampleExtra DH2("dh2", 1);
    
  // connect DH1 to DH2 with non-blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection2", &DH1, &DH2, &memTH, false);
    
  // initialize
  DH1.init();
  DH2.init();
  {    
    // fill the DataHolders.
    DH1.getBufferElement(0) = makefcomplex(17,-3.5);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(2);
    DH2.setCounter(0);
    // fill extra blob
    BlobOStream& bos = DH1.fillVariableBuffer();
    bos << "a string";
    // do the data transport
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    BlobIStream& bis = DH2.readVariableBuffer();
    std::string str;
    bis >> str;
    bis.getEnd();
    if (str != "a string") {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = makefcomplex(15,-4.5);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(2);
    DH2.setCounter(0);
    // do the data transport (without data in the extra blob)
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    // Extra blob should be the same as the one before.
    BlobIStream& bis = DH2.readVariableBuffer();
    std::string str;
    bis >> str;
    bis.getEnd();
    if (str != "a string") {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = makefcomplex(151,-4.5);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(2);
    DH2.setCounter(0);
    // make empty extra blob
    DH1.clearVariableBuffer();
    // do the data transport (without data in the extra blob)
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    bool found;
    int version;
    DH2.readVariableBuffer(found, version);
    if (found) {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = makefcomplex(151,-41.5);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(2);
    DH2.setCounter(0);
    // do the data transport (without data in the extra blob)
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    bool found;
    int version;
    DH2.readVariableBuffer(found, version);
    if (found) {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = makefcomplex(1.7,3.52);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(5);
    DH2.setCounter(0);
    BlobOStream& bos = DH1.fillVariableBuffer();
    bos << int(1) << float(3);
    bos.putStart ("p3", 3);
    bos.putEnd();
    // do the data transport
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    BlobIStream& bis = DH2.readVariableBuffer();
    int v1;
    float v2;
    bis >> v1 >> v2;
    int vers = bis.getStart ("p3");
    bis.getEnd();
    bis.getEnd();
    if (v1 != 1  ||  v2 != 3  ||  vers != 3) {
      return false;
    }

  }
  return true;
}

bool testReset()   // This tests the reset functionality needed by CEPFrame
{
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
  DH_Example DH3("dh3", 1);
    
  // connect DH1 to DH2 with non-blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection1", &DH1, &DH2, &memTH, false);
    
  // initialize
  DH1.init();
  DH2.init();
  DH3.init();
    
  // fill the DataHolders with some initial data
  DH1.getBufferElement(0) = makefcomplex(17,-3.5);
  DH2.getBufferElement(0) = makefcomplex(0,0);
  DH1.setCounter(1);
  DH2.setCounter(0);
    
  cout << "Before transport : DH1 " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- DH2 " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);
  
  cout << "After transport : DH1 " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- DH2 " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;


  if (DH1.getBufferElement(0) != DH2.getBufferElement(0)
      ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect" << endl;
    return false;
  }

  conn.setSourceDH(&DH3);
  memTH.reset();

  // fill the DataHolders with some initial data
  DH3.getBufferElement(0) = makefcomplex(8,-4.5);
  DH1.getBufferElement(0) = makefcomplex(1,-7.1);
  DH3.setCounter(2);
  DH1.setCounter(3);
    
  cout << "Before transport : DH1 " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- DH2 " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << " -- DH3 " 
       << DH3.getBufferElement(0) << ' ' << DH3.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);
  
  cout << "After transport : DH1 " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- DH2 " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << " -- DH3 " 
       << DH3.getBufferElement(0) << ' ' << DH3.getCounter()
       << endl;


  if (DH2.getBufferElement(0) != DH3.getBufferElement(0)
      ||  DH2.getCounter() != DH3.getCounter()) {
    cout << "Data in receiving DataHolder3 is incorrect" << endl;
    return false;
  }

  return true;
}

int main()
{
  bool result = true;
  try {
    cout << "Transport Example test program" << endl;
    cout << "test1 ..." << endl;
    result &= test1();
    cout << "test2 ..." << endl;
    result &= test2();
    cout << "testReset ..." << endl;
    result &= testReset();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}
