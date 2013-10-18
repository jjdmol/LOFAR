//# ExampleNonBlocking.cc: Example program to illustrate the principle (and 
//# danger!) of non-blocking communication
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

#include "DH_Example.h"
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;

#ifdef USE_THREADS

void* startWriterThread(void* thread_arg)
{
  Connection* conn = (Connection*)thread_arg;
  DH_Example* dh   = (DH_Example*)conn->getDataHolder();
  dh->getBufferElement(0) = makefcomplex(17,-3.5);
  dh->setCounter(1);
  cout << "Sending buffer[0] = " << dh->getBufferElement(0)
       << "  counter = " << dh->getCounter() << endl;
  conn->write();
  // Modify the dataholder before the previous write has completely sent
  // the data.
  dh->getBufferElement(0) = makefcomplex(2,8);
  dh->setCounter(2);

  usleep(3000000); // Wait

  pthread_exit(NULL);
}

#endif

bool test1()
{
#ifdef USE_THREADS
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // connect DH1 to DH2 with non-blocking in-memory communication
  TH_Mem memTH;
  Connection conn("connection1", &DH1, &DH2, &memTH, false);
    
  // initialize
  DH1.init();
  DH2.init();

  DH2.getBuffer()[0] = makefcomplex(0,0);
  DH2.setCounter(0);
  
  // Create a writing thread
  pthread_t writer;
  if (pthread_create(&writer, NULL, startWriterThread, &conn))
  {
    cout << "Thread creation failed" << endl;
  }

  usleep(3000000); // Wait

  // Read
  bool result = true;
  conn.read();
  cout << "Received buffer[0] = " << DH2.getBufferElement(0)
       << "  counter = " << DH2.getCounter() << endl;

  // The read data is not equal to the sent data, but to the modified value!
  if (DH2.getBufferElement(0) != makefcomplex(2, 8) ||
	DH2.getCounter() != 2)
  {
      result = false;
  }

  pthread_join(writer, NULL);

  cout << "test1 result: " << result << endl;
  return result;
#else
  return true;
#endif
}


int main(int, const char**)
{
  bool result = true;
  try {
    INIT_LOGGER("ExampleNonBlocking.log_prop");
    cout << "Transport ExampleNonBlocking test program" << endl;
    cout << "test1 ..." << endl;
    result &= test1();
    } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}
