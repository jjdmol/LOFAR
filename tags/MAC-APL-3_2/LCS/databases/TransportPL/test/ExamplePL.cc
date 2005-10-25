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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <DH_Example.h>
#include <DH_ExampleExtra.h>
#include <TransportPL/TH_PL.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/BlobOStream.h>
#include <Common/BlobArray.h>
#include <vector>
#include <iostream>

using namespace LOFAR;


void sendData1 (Connection& conn)
{
  DH_Example* sender = (DH_Example*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  conn.write();
}

void sendExtraData1 (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  conn.write();
}

void sendData2 (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();  
  sender->getBuffer()[0] = makefcomplex(17,-3.5);
  sender->setCounter(2);
  // fill extra blob (> 1 KByte because of possible DTL bug)
  BlobOStream& bos = sender->fillVariableBuffer();
  bos << "a string";
  bos << std::vector<int>(256,1);
  conn.write();
}

void sendData3 (Connection& conn)
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

void sendData4 (Connection& conn)
{
  DH_ExampleExtra* sender = (DH_ExampleExtra*)conn.getDataHolder();
  sender->getBuffer()[0] = makefcomplex(1.7,3.52);
  sender->setCounter(5);
  conn.write();
}

void receiveData (Connection& conn, Connection& connRes)
{
  DH_Example* receiver = (DH_Example*)conn.getDataHolder(true);
  DH_Example* result = (DH_Example*)connRes.getDataHolder(true);

  connRes.read();
  conn.read();
  cout << "Received " << receiver->getDataSize() << " bytes" << endl;
  ASSERT (receiver->getDataSize() == result->getDataSize());

  const char* d1 = static_cast<char*>(result->getDataPtr());
  const char* d2 = static_cast<char*>(receiver->getDataPtr());
  for (int i=0; i<result->getDataSize(); i++) {
    ASSERT (d1[i] == d2[i]);
  }
}

void receiveExtraData (Connection& conn, Connection& connRes)
{
  DH_ExampleExtra* receiver = (DH_ExampleExtra*)conn.getDataHolder(true);
  DH_ExampleExtra* result = (DH_ExampleExtra*)connRes.getDataHolder(true);

  connRes.read();
  conn.read();
  cout << "Received " << receiver->getDataSize() << " bytes" << endl;
  ASSERT (receiver->getDataSize() == result->getDataSize());
  const char* d1 = static_cast<char*>(result->getDataPtr());
  const char* d2 = static_cast<char*>(receiver->getDataPtr());
  for (int i=0; i<result->getDataSize(); i++) {
    ASSERT (d1[i] == d2[i]);
  }
}

void test1()
{
  DH_Example DH_Sender("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);

  TH_PL plTH("ExamplePL");
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &plTH);
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_PL receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  cout << "Send data1" << endl;
  sendData1 (conn1);
  sendData1 (conn2);
  receiveData (conn1, conn2);
}

void test2()
{
  DH_ExampleExtra DH_Sender("dh1", 1);
  DH_ExampleExtra DH_Receiver("dh2", 1);
  TH_PL plTH("ExamplePL");
  Connection conn1("connection1", &DH_Sender, &DH_Receiver, &plTH);
  DH_Sender.init();
  DH_Receiver.init();

  DH_ExampleExtra dh1("dh1mem", 1);
  DH_ExampleExtra dh2("dh2mem", 1);
  TH_Mem memTH;
  Connection conn2("connection2", &dh1, &dh2, &memTH, false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_PL receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  cout << "Send data1" << endl;
  sendExtraData1 (conn1);
  sendExtraData1 (conn2);
  receiveExtraData (conn1, conn2);
  cout << "Send data2" << endl;
  sendData2 (conn1);
  sendData2 (conn2);
  receiveExtraData (conn1, conn2);
  cout << "Send data3" << endl;
  sendData3 (conn1);
  sendData3 (conn2);
  receiveExtraData (conn1, conn2);
  cout << "Send data4" << endl;
  sendData4 (conn1);
  sendData4 (conn2);
  receiveExtraData (conn1, conn2);
}


int main (int argc, const char** argv)
{
  INIT_LOGGER("ExamplePL.log_prop");

  cout << "Transport ExamplePL test program" << endl;
  int test=1;
  if (argc > 1  &&  std::string(argv[1]) == "2") {
    test = 2;
  }

  TH_PL::useDatabase ("test");

  try {
    if (test == 1) {
      test1();
    } else {
      test2();
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
