//#  ExampleEthernet.cc: a test program for the TH_Ethernet class
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

#include "DH_Ethernet.h"
#include <Transport/TH_Ethernet.h>
#include <Transport/TH_Mem.h>
#include <Common/BlobOStream.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/Timer.h>
#include <iostream>
#include <stdlib.h>

using namespace LOFAR;


void sendData (DH_Ethernet& sender)
{
  for (int i=0;i<368;i++) {
    if (i <3 || i > 364) sender.getBuffer()[i] = '*';
    else sender.getBuffer()[i] = '-';
  }
  sender.write();
  if (sender.getID() == 1) cout << "sent " << sender.getDataSize() << " bytes" << endl; 
}

void receiveData (DH_Ethernet& receiver, DH_Ethernet& result)
{
  result.read();
  receiver.read();
  ASSERT(receiver.getDataSize() == result.getDataSize());
  cout << "Received " << result.getDataSize() << " bytes" << endl;
  const char* data1 = static_cast<char*>(result.getDataPtr());
  const char* data2 = static_cast<char*>(receiver.getDataPtr());
  
  for (int i=0; i<receiver.getDataSize(); i++) {
    ASSERT(data1[i] == data2[i]);
  }
}

void test (bool isReceiver, string interface, string remoteMac, string ownMac)
{
  DH_Ethernet DH_Sender("dh_sender", 368);
  DH_Ethernet DH_Receiver("dh_receiver", 368);
  DH_Sender.setID(1);
  DH_Receiver.setID(2);
  TH_Ethernet proto(interface, remoteMac, ownMac, 0x000, false);
  
  DH_Sender.connectTo (DH_Receiver, proto, true);
  if (isReceiver) { 
    ASSERT(DH_Receiver.init() == true);
  }
  else { 
    ASSERT(DH_Sender.init() == true);
  }

  //Use a TH_Mem to check if the ethernet receiver gets the correct data
  DH_Ethernet dh1("dh1mem",368);
  DH_Ethernet dh2("dh2mem",368);
  dh1.setID(3);
  dh2.setID(4);
  dh1.connectTo(dh2, TH_Mem(), false);
  dh1.init();
  dh2.init();

  if (isReceiver) {  
    sendData (dh1);   
    receiveData (DH_Receiver, dh2);
  } else {
    cout << "Send data1" << endl;
    sendData(DH_Sender);
  }
}

int main (int argc, const char** argv)
{
  INIT_LOGGER("ExampleEthernet.log_prop");
  
  string which;
  
  try {
    bool isReceiver; 
    string ownMac;
    
    if (argc < 4) {
      cout << "Usage: ExampleEthernet [-s|-r] [interface] [remoteMac] <ownMac>" << endl;
      cout << "Skipping test" << endl;
      return 1;
    }
    if (argc < 5) ownMac = "";
    else ownMac = (string)argv[4];
   
    if (!strcmp(argv[1], "-s")) {
      isReceiver = false;
      which = "Sender";
    } else if (!strcmp(argv[1], "-r")) {
      cout << "(Listening...)" << endl; 
      isReceiver = true;
      which = "Receiver";
    }
    

    // Execute test
    test(isReceiver, (string)argv[2], (string)argv[3], ownMac);
     
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << which << ": " << x.what() << endl;
    return 1;
  }
  cout << which << " OK" << endl;
  return 0;
}


