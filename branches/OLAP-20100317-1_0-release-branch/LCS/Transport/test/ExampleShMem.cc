//# ExampleShMem.cc: Test program for class TH_ShMem
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

// //# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <DH_Example.h>
#include <DH_ExampleExtra.h>
#include <Transport/TH_ShMem.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Blob/BlobOStream.h>
#include <iostream>

using namespace LOFAR;

#if defined(HAVE_MPI)  &&  defined(HAVE_SHMEM)

// This test program is meant for testing ShMem in an MPI environment.

void sendData1 (Connection& conn)
{
  DH_Example* sender = (DH_Example*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  cout << "Sending counter " << sender->getCounter() << endl;
  conn.write();
}

void receiveData1 (Connection& conn1, Connection& connMem)
{
  DH_Example* receiver = (DH_Example*)conn1.getDataHolder(true);
  DH_Example* result = (DH_Example*)connMem.getDataHolder(true);
  connMem.read();
  conn1.read();
  cout << "Received " << receiver->getDataSize() << " bytes" << endl;
  ASSERT (receiver->getDataSize() == result->getDataSize());
  const char* d1 = static_cast<char*>(result->getDataPtr());
  const char* d2 = static_cast<char*>(receiver->getDataPtr());
  for (int i=0; i<result->getDataSize(); i++) {
    ASSERT (d1[i] == d2[i]);
  }
  cout << "Received counters " << result->getCounter() << " == " << receiver->getCounter() << endl;
  cout << "receiveData1 succeeded" << endl;
}

void sendDataA (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  conn.write();
}

void sendDataB (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  // fill extra blob
  BlobOStream& bos = sender->fillVariableBuffer();
  bos << "a string";
  conn.write();
}

void sendDataC (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(15,-4.5);
  sender->setCounter(5);
  BlobOStream& bos = sender->fillVariableBuffer();
  bos << int(1) << float(3);
  bos.putStart ("p3", 3);
  bos.putEnd();
  conn.write();
}

void sendDataD (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(1.7,3.52);
  sender->setCounter(5);
  conn.write();
}

void receiveData2 (Connection& conn1, Connection& connMem)
{
  DH_ExampleExtra* receiver = (DH_ExampleExtra*)conn1.getDataHolder(true);
  DH_ExampleExtra* result = (DH_ExampleExtra*)connMem.getDataHolder(true);
  connMem.read();
  conn1.read();
  cout << "Received " << receiver->getDataSize() << " bytes" << endl;
  ASSERT (receiver->getDataSize() == result->getDataSize());
  const char* d1 = static_cast<char*>(result->getDataPtr());
  const char* d2 = static_cast<char*>(receiver->getDataPtr());
  for (int i=0; i<result->getDataSize(); i++) {
    ASSERT (d1[i] == d2[i]);
  }
  cout << "receiveData1 succeeded" << endl;
}

void test1 (bool isReceiver)
{
  DH_Example DH_Sender("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);
  TH_ShMem shMem(new TH_MPI(0,1));
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &shMem, true);
  shMem.init();
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_ShMem receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendData1 (conn2);
    receiveData1 (conn1, conn2);
  } else {
    cout << "Send data1" << endl;
    sendData1 (conn1);
    sleep(1);
  }
}

void test2 (bool isReceiver, int maxSize)
{
  DH_ExampleExtra DH_Sender("dh1", 1);
  DH_Sender.setMaxDataSize (maxSize, true);
  DH_ExampleExtra DH_Receiver("dh2", 1);
  DH_Receiver.setMaxDataSize (maxSize, true);
  TH_ShMem shMem(new TH_MPI(0,1));
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &shMem, true);
  shMem.init();
  DH_Sender.init();
  DH_Receiver.init();

  DH_ExampleExtra dh1("dh1mem", 1);
  DH_ExampleExtra dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_ShMem receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendDataA (conn2);
    receiveData2 (conn1, conn2);
    sendDataB (conn2);
    receiveData2 (conn1, conn2);
    sendDataC (conn2);
    receiveData2 (conn1, conn2);
    sendDataD (conn2);
    receiveData2 (conn1, conn2);
  } else {
    cout << "Send data1" << endl;
    sendDataA (conn1);
    cout << "Send data2" << endl;
    sendDataB (conn1);
    cout << "Send data3" << endl;
    sendDataC (conn1);
    cout << "Send data4" << endl;
    sendDataD (conn1);
  }
}

void testReset (bool isReceiver)
{
  cout << "In testReset() " << endl;
  DH_Example DH_Sender1("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);
  TH_ShMem shMem(new TH_MPI(0,1));
  Connection conn1("connection1", &DH_Sender1, &DH_Receiver, &shMem, true);
  shMem.init();
  DH_Sender1.init();
  DH_Receiver.init();
  DH_Sender1.setCounter(8);
  DH_Receiver.setCounter(7);

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();
  dh1.setCounter(8);
  dh2.setCounter(7);

  // Use a TH_Mem to check if the TH_ShMem receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendData1 (conn2);
    receiveData1 (conn1, conn2);

    // reconnect to 3rd dh
    DH_Example* dh3 = dh1.clone();  
    dh3->init();
    dh3->setCounter(66);
    conn2.setSourceDH(dh3);
    memTH.reset();
    sendData1 (conn2);
    receiveData1 (conn1, conn2);

  } else {
    cout << "Send data1" << endl;
    sendData1 (conn1);

    // reconnect to 3rd dh
    DH_Example* DH_Sender3 = DH_Sender1.clone();  // Use clone method to make sure data is
    DH_Sender3->init();                           // allocated in shared memory
    DH_Sender3->setCounter(66);
    conn1.setSourceDH(DH_Sender3); 
    shMem.reset();
    cout << "Send data3" << endl;
    sendData1 (conn1);

  }
}

int main (int argc, char** argv)
{
  int test=1;
  if (argc > 1  &&  std::string(argv[1]) == "2") {
    test = 2;
  }
  if (argc > 1  &&  std::string(argv[1]) == "3") {
    test = 3;
  }
  INIT_LOGGER("ExampleShMem.log_prop");
  TH_MPI::initMPI (argc, argv);

  MPI_Bcast (&test, 1, MPI_INT, 0, MPI_COMM_WORLD);

  bool isReceiver; 
  string which;
  try {
    // isReceiver == false => this program must run as a server (receiver).
    // isReceiver == true  => this program must run as a client (sender).

   if (TH_MPI::getCurrentRank() == 0) {
      cout << "(Server side)" << endl;
      isReceiver = false;
      which = "sender";
    } else {
      cout << "(Client side)" << endl;
      isReceiver = true;
      which = "receiver";
    }
 
    cout << "Transport ExampleShMem test program " << test << endl;

    if (test == 1) {
      test1(isReceiver);
    } else if (test == 2) {
      test2(isReceiver, 1000);
    } else if (test == 3) {
      testReset(isReceiver);
    } else {
      try {
	test2(isReceiver, 0);
      } catch (std::exception& x) {
	cout << "Caught expected exception: " << x.what() << endl;
      }
    }

    TH_MPI::finalize();
 
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << which << ": " << x.what() << endl;
    return 1;
  }
  cout << which << " OK" << endl;
  return 0;
}

#else

int main()
{
  cout << "MPI is not installed" << endl;
  return 0;
}

#endif
