//# ExampleNonBlocking.cc: Example program to illustrate the principle (and 
//#                        danger!) of non-blocking communication
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
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;

#ifdef USE_THREADS

void* startWriterThread(void* thread_arg)
{
  DH_Example* dh = (DH_Example*)thread_arg;
  dh->getBufferElement(0) = fcomplex(17,-3.5);
  dh->setCounter(1);
  cout << "Sending buffer[0] = " << dh->getBufferElement(0)
       << "  counter = " << dh->getCounter() << endl;
  dh->write();
  // Modify the dataholder before the previous write has completely sent
  // the data.
  dh->getBufferElement(0) = fcomplex(2,8);
  dh->setCounter(2);

  pthread_exit(NULL);
}

#endif

bool test1()
{
#ifdef USE_THREADS
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

  DH2.getBuffer()[0] = 0;
  DH2.setCounter(0);
  
  // Create a writing thread
  pthread_t writer;
  if (pthread_create(&writer, NULL, startWriterThread, &DH1))
  {
    cout << "Thread creation failed" << endl;
  }

  usleep(3000000); // Wait

  // Read
  bool result = true;
  DH2.read();
  cout << "Received buffer[0] = " << DH2.getBufferElement(0)
       << "  counter = " << DH2.getCounter() << endl;

  // The read data is not equal to the sent data, but to the modified value!
  if (DH2.getBufferElement(0) != fcomplex(2, 8) ||
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


int main(int argc, const char** argv)
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
