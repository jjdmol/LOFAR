//# ExampleMem.cc: Test program for basic Transport classes
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

#include <DH_Example.h>
#include <Transport/TH_Mem.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <iostream>

using namespace LOFAR;

bool test1()
{
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // Assign an ID for each dataholder by hand for now
  // This will be done by the framework later on
  DH1.setID(1);
  DH2.setID(2);

  // connect DH1 to DH2 with non-blocking in-memory communication
  DH1.connectTo(DH2, TH_Mem(), false);
    
  // initialize
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBufferElement(0) = fcomplex(17,-3.5);
  DH2.getBufferElement(0) = 0;
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  DH1.write();
  DH2.read();
  
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
  DH_Example DH1("dh1", 1, true);
  DH_Example DH2("dh2", 1, true);
    
  // Assign an ID for each dataholder by hand for now
  // This will be done by the framework later on
  DH1.setID(1);
  DH2.setID(2);

  // connect DH1 to DH2 with non-blocking in-memory communication
  DH1.connectTo(DH2, TH_Mem(), false);
    
  // initialize
  DH1.init();
  DH2.init();
  {    
    // fill the DataHolders.
    DH1.getBufferElement(0) = fcomplex(17,-3.5);
    DH2.getBufferElement(0) = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    // fill extra blob
    BlobOStream& bos = DH1.createExtraBlob();
    bos << "a string";
    // do the data transport
    DH1.write();
    DH2.read();
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    int version;
    bool found;
    BlobIStream& bis = DH2.getExtraBlob(found, version);
    if (!found) {
      return false;
    }
    std::string str;
    bis >> str;
    bis.getEnd();
    if (str != "a string") {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = fcomplex(15,-4.5);
    DH2.getBufferElement(0) = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    // do the data transport (without data in the extra blob)
    DH1.write();
    DH2.read();
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    // Extra blob should be the same as the one before.
    int version;
    bool found;
    BlobIStream& bis = DH2.getExtraBlob(found, version);
    if (!found) {
      return false;
    }
    std::string str;
    bis >> str;
    bis.getEnd();
    if (str != "a string") {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = fcomplex(151,-4.5);
    DH2.getBufferElement(0) = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    // make empty extra blob
    DH1.clearExtraBlob();
    // do the data transport (without data in the extra blob)
    DH1.write();
    DH2.read();
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    int version;
    bool found;
    DH2.getExtraBlob(found, version);
    if (found) {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = fcomplex(151,-41.5);
    DH2.getBufferElement(0) = 0;
    DH1.setCounter(2);
    DH2.setCounter(0);
    // do the data transport (without data in the extra blob)
    DH1.write();
    DH2.read();
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    int version;
    bool found;
    DH2.getExtraBlob(found, version);
    if (found) {
      return false;
    }
  }
  {
    DH1.getBufferElement(0) = fcomplex(1.7,3.52);
    DH2.getBufferElement(0) = 0;
    DH1.setCounter(5);
    DH2.setCounter(0);
    BlobOStream& bos = DH1.createExtraBlob();
    bos << int(1) << float(3);
    bos.putStart ("p3", 3);
    bos.putEnd();
    // do the data transport
    DH1.write();
    DH2.read();
    if (! (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	   &&  DH1.getCounter() == DH2.getCounter())) {
      return false;
    }
    int version;
    bool found;
    BlobIStream& bis = DH2.getExtraBlob(found, version);
    if (!found) {
      return false;
    }
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

int main()
{
  bool result = true;
  try {
    cout << "Transport Example test program" << endl;
    cout << "test1 ..." << endl;
    result &= test1();
    cout << "test2 ..." << endl;
    result &= test2();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}
