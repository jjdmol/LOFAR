//# ExampleMPI.cc: a test program for the TH_MPI class
//#
//# Copyright (C) 2002-2003
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
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR;

#ifdef HAVE_MPI

void sendData1 (Connection& conn)
{
  DH_Example* sender = (DH_Example*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  conn.write();
}

void sendData2 (Connection& conn)
{
  DH_Example* sender = (DH_Example*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(1.7,3.52);
  sender->setCounter(5);
  conn.write();
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
  cout << "receiveData1 succeeded" << endl;
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

void sendDataBDTest1 (DH_Example& sender, Connection& conn)
{
  sender.getBuffer()[0] = makefcomplex(17,-3.5);
  sender.setCounter(2);
  conn.write();
}

void sendDataBDTest2 (DH_Example& sender, Connection& conn)
{
  sender.getBuffer()[0] = makefcomplex(1.7,3.52);
  sender.setCounter(5);
  conn.write();
}

void receiveDataBDTest (DH_Example& receiver, Connection& connRec, 
			DH_Example& result, Connection& connRes)
{
  connRes.read();
  connRec.read();
  cout << "Received " << receiver.getDataSize() << " bytes" << endl;
  ASSERT (receiver.getDataSize() == result.getDataSize());
  const char* d1 = static_cast<char*>(result.getDataPtr());
  const char* d2 = static_cast<char*>(receiver.getDataPtr());
  for (int i=0; i<result.getDataSize(); i++) {
    ASSERT (d1[i] == d2[i]);
  }
}

void test1 (bool isReceiver)
{
  DH_Example DH_Sender("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);
  TH_MPI mpiTH(0,1);
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &mpiTH, true);
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_MPI receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendData1 (conn2);
    receiveData1 (conn1, conn2);
  } else {
    cout << "Send data1" << endl;
    sendData1 (conn1);
  }
}

void test2 (bool isReceiver)
{
  DH_ExampleExtra DH_Sender("dh1", 1);
  DH_ExampleExtra DH_Receiver("dh2", 1);
  TH_MPI mpiTH(0,1);
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &mpiTH, true);
  DH_Sender.init();
  DH_Receiver.init();

  DH_ExampleExtra dh1("dh1mem", 1);
  DH_ExampleExtra dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_MPI receiver gets the correct data.
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

void testBidirectional (bool isReceiver)
{
  DH_Example DH_Sender("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);
  TH_MPI mpiTH1(0,1);
  TH_MPI mpiTH2(1,0);
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &mpiTH1, true);
  Connection conn2("connection2", &DH_Receiver, &DH_Sender, &mpiTH2, true);
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  TH_Mem memTH1;
  TH_Mem memTH2;
  Connection conn3("connection3", &dh1, &dh2, &memTH1, false);
  Connection conn4("connection4", &dh2, &dh1, &memTH2, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_MPI receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendDataBDTest1 (dh1, conn3);
    receiveDataBDTest (DH_Receiver, conn1, dh2, conn3);
    // And send different data back
    cout << "Receiver sending data4 back" << endl;
    sendDataBDTest2(DH_Receiver, conn2);
  } else {
    cout << "Send data1" << endl;
    sendDataBDTest1 (DH_Sender, conn1);
    // And receive data back
    sendDataBDTest2(dh2, conn4);
    receiveDataBDTest (DH_Sender, conn2, dh1, conn4);
  }
}


int main (int argc, char** argv)
{
  INIT_LOGGER("ExampleMPI.log_prop");
  TH_MPI::initMPI (argc, argv);

  string which;
  try {
    bool isReceiver; 
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

    test1(isReceiver);
    test2(isReceiver);
    testBidirectional(isReceiver);
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
