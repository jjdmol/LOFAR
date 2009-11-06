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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <DH_VarBuf.h>
#include <Transport/Connection.h>
#include <Transport/TH_Mem.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <iostream>

using namespace LOFAR;

void fillDataHolder(DH_VarBuf& dh, unsigned int size, float factor)
{
  dh.setBufferSize(size);
  fcomplex* dataPtr = dh.getBuffer();
  float value = 0;
  for (unsigned int i=0; i<size; i++)
  {
    value = factor*i;
    *(dataPtr+i) = makefcomplex(value, value+1);
  }
}

bool test()
{
  bool result = false;

  DH_VarBuf DH1("dh1");
  DH_VarBuf DH2("dh2");
    
  // connect DH1 to DH2 with non-blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection1", &DH1, &DH2, &memTH, false);
    
  // initialize
  DH1.init();
  DH2.init();

  // fill the sending DataHolder with some initial data
  fillDataHolder(DH1, 10, 1);
  fillDataHolder(DH2, 1, 0.1);
    
  DH1.setCounter(1);
  DH2.setCounter(0);
  
  unsigned int dh1Size = DH1.getBufferSize();
  unsigned int dh2Size = DH2.getBufferSize();
  
  cout << "Before transport 1: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".." 
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) << ".." 
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);

  dh1Size = DH1.getBufferSize();
  dh2Size = DH2.getBufferSize();
  
  cout << "After transport 1: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".." 
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) << ".."
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
      &&  DH1.getCounter() == DH2.getCounter() 
      && DH1.getBufferSize() == DH2.getBufferSize()
      && DH1.getBufferElement(dh1Size-1) == DH2.getBufferElement(dh2Size-1)) {
    result = true;
  }
  else
  {
    cout << "Data in receiving DataHolder is incorrect" << endl;
  }

  // fill the sending DataHolder with some other data
  fillDataHolder(DH1, 100, 1.5);
    
  DH1.setCounter(2);

  dh1Size = DH1.getBufferSize();
  dh2Size = DH2.getBufferSize();
    
  cout << "Before transport 2: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".."
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) << ".." 
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);

  dh1Size = DH1.getBufferSize();
  dh2Size = DH2.getBufferSize();
  
  cout << "After transport 2: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".." 
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) << ".." 
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
      &&  DH1.getCounter() == DH2.getCounter() 
      && DH1.getBufferSize() == DH2.getBufferSize()
      && DH1.getBufferElement(99) == DH2.getBufferElement(99)) {
    result &= true;
  }
  else
  {
    cout << "Data in receiving DataHolder is incorrect" << endl;
  }

  // fill the sending DataHolder with some other data
  fillDataHolder(DH1, 3, .2);
    
  DH1.setCounter(3);

  dh1Size = DH1.getBufferSize();
  dh2Size = DH2.getBufferSize();
    
  cout << "Before transport 3: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".." 
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) <<  ".." 
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  ASSERT(conn.write() == Connection::Finished);
  ASSERT(conn.read() == Connection::Finished);

  dh1Size = DH1.getBufferSize();
  dh2Size = DH2.getBufferSize();
  
  cout << "After transport 3: " 
       << dh1Size << ' ' << DH1.getBufferElement(0) << ".." 
       << DH1.getBufferElement(dh1Size-1) << ' ' << DH1.getCounter()
       << " -- " 
       << dh2Size << ' ' << DH2.getBufferElement(0) << ".." 
       << DH2.getBufferElement(dh2Size-1) << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
      &&  DH1.getCounter() == DH2.getCounter() 
      && DH1.getBufferSize() == DH2.getBufferSize()
      && DH1.getBufferElement(2) == DH2.getBufferElement(2)) {
    result &= true;
  }
  else
  {
    cout << "Data in receiving DataHolder is incorrect" << endl;
  }

  return result;
}


int main()
{
  bool result = true;
  try {
    cout << "Transport ExampleVarBuf test program" << endl;
    cout << "test ..." << endl;
    result &= test();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}
