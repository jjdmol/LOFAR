//# ExampleSocket.cc: a test program for the TH_Socket class
//# this program sends a series of packages 
//# with increasing package size and measures
//# the transport bandwidth
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

#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>
#include <Transport/TH_Socket.h>
#include "DH_Example.h"
#include "StopWatch.h"
#include <iomanip>

using namespace LOFAR;

void displayUsage (void);

int main (int argc, char** argv) {
  bool isReceiver; 
  // isReceiver == true  => this program must run as a server (receiver).
  // isReceiver == false => this program must run as a client (sender).

  cout << "argc = " << argc << endl;
  if (argc < 3) {
    displayUsage ();
    return 0;
  }
  cout << "argc = " << argc << endl;

  bool ServerAtSender=false; // flag for who should be the server

  if (! strcmp (argv [1], "-r")) {
    cout << "(Receiver side)" << endl;
    isReceiver = true;
    if (argc >= 3  && !strcmp(argv[2], "-Server")) ServerAtSender=false;
    if (argc >= 3  && !strcmp(argv[2], "-Client")) ServerAtSender=true;
  } else if (! strcmp (argv [1], "-s")) {
    cout << "(Sender side)" << endl;
    isReceiver = false;
    if (argc >= 3 && !strcmp(argv[2], "-Server"))  ServerAtSender=true;
    if (argc >= 3 && !strcmp(argv[2], "-Client"))  ServerAtSender=false;
  } else {
    displayUsage ();
    return 0;
  }

  INIT_LOGGER(argv[0]);

  const int maxlen = 1024*1024*1;
  DH_Example DH_Sender("dh1", maxlen);
  DH_Example DH_Receiver("dh", maxlen);

  DH_Sender.setID(1);
  DH_Receiver.setID(2);

  TH_Socket TH_proto("localhost", 
		     "localhost",
		     8923,
		     !ServerAtSender); //  set the server side

  DH_Sender.connectTo (DH_Receiver, TH_proto, true);
  if (isReceiver) {
    DH_Receiver.init();
  }
  else {
    DH_Sender.init();
  }
 
  if (isReceiver) {
    DH_Receiver.getBuffer()[0] = makefcomplex(0,0);
    DH_Receiver.getBuffer()[1] = makefcomplex(0,0);
    DH_Receiver.setCounter(0);
    cout << "Before transport : " ;
	cout << DH_Receiver.getBuffer()[1] << ' ' << DH_Receiver.getCounter() << endl;
  }
  else {
    // fill the DataHolders with some initial data
    DH_Sender.getBuffer()[0] = makefcomplex(17,-3.5);
    DH_Sender.getBuffer()[1] = makefcomplex(18,-2.5);
    DH_Sender.setCounter(2);
    cout << "Before transport : " ;
	cout << DH_Sender.getBuffer()[1] << ' ' << DH_Sender.getCounter() << endl;
  }


  StopWatch watch;
  watch.stop();


  int sizes[10000];
  double times[10000];
  int i=0;

  for (int l=1; l<maxlen; l= (int)ceil(1.051*l)) {
    ///  for (int l=1; l<maxlen; l*=2) {

    // resize
    if (isReceiver) {
      DH_Receiver.setBufferSize(l);
      sizes[i] = DH_Receiver.getDataSize();
    } else {
      DH_Sender.setBufferSize(l);
      sizes[i] = DH_Sender.getDataSize();
    }
    
    // perform measurement
    // 10-1000 iterations are done depending on the pkg size
    // the average transport time is calculated on the send side
    int measurements=10;
    if (l< 256*1024) measurements = 100;
    if (l<  16*1024) measurements = 1000;
    if (l<   1*1024) measurements = 10000;
    //    if (l ==     10) measurements = 100000000;

    //    cout << "len = " << l << "  meas: " << measurements << endl;
    cout << "msg length = " << l << "bytes" 
	 << "                       \r" << flush;
    
    if (!isReceiver) watch.start();
    for (int m=0; m< measurements; m++) {
      if (isReceiver) {
        DH_Receiver.read();
      } else {
	DH_Sender.write();
      }
    }
    if (!isReceiver) {
      watch.stop();
      times[i++] = watch.elapsed()/measurements;
    }
  }
  
   if (!isReceiver)  {
    ofstream outFile("ExampleSocketPerf_tmp.bandwidth_dat");
    for (int m=0; m<i; m++) {
      outFile << std::setw(8) << sizes[m]           // packet size in bytes/
	      << std::setw(14) << log(double(sizes[m])) // log packet size 
	      << std::setw(14) << times[m]          // time interval
	      << std::setw(10) << sizes[m]/1024./1024./times[m]  // bandwidth in MB/sec
	      << endl;
    }
    outFile.close();
  }
  
  if (isReceiver) {
    cout << " After transport  : " 
	 << DH_Sender.getBuffer()[1] << ' ' << DH_Sender.getCounter()
	 << " -- " 
	 << DH_Receiver.getBuffer()[1] << ' ' << DH_Receiver.getCounter()
	 << endl;
    /*
      if (DH_Sender.getBuffer()[0] == DH_Receiver.getBuffer()[0]
      &&  DH_Sender.getCounter() == DH_Receiver.getCounter()) {
      }
    */
  }
  return 0;
}


void displayUsage (void) {
    cout << "Usage: ExampleSocketPerf -s|-r [-Server|-Client]" << endl;
    cout << "If Client/Server flags are not specified, the receiver side will be Server" << endl;
    cout << "(exit)." << endl;
}
