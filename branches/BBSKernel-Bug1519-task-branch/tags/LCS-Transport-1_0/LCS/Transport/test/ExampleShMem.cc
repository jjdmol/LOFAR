//# ExampleShMem.cc: Test program for class TH_ShMem
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
#include <Transport/TH_ShMem.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <Common/BlobOStream.h>
#include <iostream>

using namespace LOFAR;

// This test program is meant for testing ShMem in an MPI environment.

#ifdef HAVE_MPI

void sendData1 (DH_Example& sender)
{
  sender.getBuffer()[0] = fcomplex(17,-3.5);
  sender.setCounter(2);
  sender.write();
}

void sendData2 (DH_Example& sender)
{
  sender.getBuffer()[0] = fcomplex(17,-3.5);
  sender.setCounter(2);
  // fill extra blob
  BlobOStream& bos = sender.createExtraBlob();
  bos << "a string";
  sender.write();
}

void sendData3 (DH_Example& sender)
{
  sender.getBuffer()[0] = fcomplex(15,-4.5);
  sender.setCounter(5);
  BlobOStream& bos = sender.createExtraBlob();
  bos << int(1) << float(3);
  bos.putStart ("p3", 3);
  bos.putEnd();
  sender.write();
}

void sendData4 (DH_Example& sender)
{
  sender.getBuffer()[0] = fcomplex(1.7,3.52);
  sender.setCounter(5);
  sender.write();
}

void receiveData (DH_Example& receiver, DH_Example& result)
{
  result.read();
  receiver.read();
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
  DH_Sender.setID(1);
  DH_Receiver.setID(2);
  DH_Sender.connectTo (DH_Receiver, TH_ShMem(0,1), true);
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1);
  DH_Example dh2("dh2mem", 1);
  dh1.setID(3);
  dh2.setID(4);
  dh1.connectTo (dh2, TH_Mem(), false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_ShMem receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendData1 (dh1);
    receiveData (DH_Receiver, dh2);
  } else {
    cout << "Send data1" << endl;
    sendData1 (DH_Sender);
  }
}

void test2 (bool isReceiver, int maxSize)
{
  DH_Example DH_Sender("dh1", 1, true);
  DH_Sender.setMaxDataSize (maxSize, true);
  DH_Example DH_Receiver("dh2", 1, true);
  DH_Receiver.setMaxDataSize (maxSize, true);
  DH_Sender.setID(1);
  DH_Receiver.setID(2);
  DH_Sender.connectTo (DH_Receiver, TH_ShMem(0,1), true);
  DH_Sender.init();
  DH_Receiver.init();

  DH_Example dh1("dh1mem", 1, true);
  DH_Example dh2("dh2mem", 1, true);
  dh1.setID(3);
  dh2.setID(4);
  dh1.connectTo (dh2, TH_Mem(), false);
  dh1.init();
  dh2.init();

  // Use a TH_Mem to check if the TH_ShMem receiver gets the correct data.
  // It should match the data sent via TH_Mem.
  if (isReceiver) {
    sendData1 (dh1);
    receiveData (DH_Receiver, dh2);
    sendData2 (dh1);
    receiveData (DH_Receiver, dh2);
    sendData3 (dh1);
    receiveData (DH_Receiver, dh2);
    sendData4 (dh1);
    receiveData (DH_Receiver, dh2);
  } else {
    cout << "Send data1" << endl;
    sendData1 (DH_Sender);
    cout << "Send data2" << endl;
    sendData2 (DH_Sender);
    cout << "Send data3" << endl;
    sendData3 (DH_Sender);
    cout << "Send data4" << endl;
    sendData4 (DH_Sender);
  }
}


int main (int argc, const char** argv)
{
  int test=1;
  if (argc > 1  &&  std::string(argv[1]) == "2") {
    test = 2;
  }
  if (argc > 1  &&  std::string(argv[1]) == "3") {
    test = 3;
  }

  INIT_LOGGER("ExampleShMem.log_prop");
  TH_ShMem::init (argc, argv);

  MPI_Bcast (&test, 1, MPI_INT, 0, MPI_COMM_WORLD);
  cout << "Transport ExampleShMem test program " << test << endl;

  string which;
  try {
    bool isReceiver; 
    // isReceiver == false => this program must run as a server (receiver).
    // isReceiver == true  => this program must run as a client (sender).

    if (TH_ShMem::getCurrentRank() == 0) {
      cout << "(Server side)" << endl;
      isReceiver = false;
      which = "sender";
    } else {
      cout << "(Client side)" << endl;
      isReceiver = true;
      which = "receiver";
    }

    if (test == 1) {
      test1(isReceiver);
    } else if (test == 2) {
      test2(isReceiver, 1000);
    } else {
      try {
	test2(isReceiver, 0);
      } catch (std::exception& x) {
	cout << "Caught expected exception: " << x.what() << endl;
      }
    }
    TH_ShMem::finalize();
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
