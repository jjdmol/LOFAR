//#  ExampleSocket.cc: a test program for the TH_Socket class
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <iostream>
#include <Transport/TH_Socket.h>
#include <DH_Example.h>

using namespace LOFAR;

void displayUsage (void);

int main (int argc, char** argv) {
  bool isReceiver; 
  // isReceiver == true  => this program must run as a server (receiver).
  // isReceiver == false => this program must run as a client (sender).

  if (argc != 2) {
    displayUsage ();
    return 0;
  }

  if (! strcmp (argv [1], "-s")) {
    cout << "(Server side)" << endl;
    isReceiver = true;
  } else if (! strcmp (argv [1], "-c")) {
    cout << "(Client side)" << endl;
    isReceiver = false;
  } else {
    displayUsage ();
    return 0;
  }

  DH_Example DH_Sender("dh1", 1);
  DH_Example DH_Receiver("dh2", 1);

  DH_Sender.setID(1);
  DH_Receiver.setID(2);

  TH_Socket proto("localhost", "localhost", 8923);

  DH_Sender.connectTo (DH_Receiver, proto, true);

  DH_Sender.init();
  DH_Receiver.init();

  if (! isReceiver) {
    // fill the DataHolders with some initial data
    DH_Sender.getBuffer()[0] = fcomplex(17,-3.5);
    DH_Receiver.getBuffer()[0] = 0;
    DH_Sender.setCounter(2);
    DH_Receiver.setCounter(0);
    cout << "Before transport : " 
	 << DH_Sender.getBuffer()[0] << ' ' << DH_Sender.getCounter()
	 << " -- " 
	 << DH_Receiver.getBuffer()[0] << ' ' << DH_Receiver.getCounter()
	 << endl;
  }
    
  if (isReceiver) {
    DH_Receiver.read();
  } else {
    DH_Sender.write();
  }


  // note that transport is bi-directional.
  // so this will also work:
  //   DH_Receiver.write();
  //   DH_Sender.read();
  // 
  
  if (isReceiver) {
    cout << "After transport  : " 
	 << DH_Sender.getBuffer()[0] << ' ' << DH_Sender.getCounter()
	 << " -- " 
	 << DH_Receiver.getBuffer()[0] << ' ' << DH_Receiver.getCounter()
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
    cout << "Usage: ExampleSocket [-s|-c]" << endl;
    cout << "(Skipping test)." << endl;
}
