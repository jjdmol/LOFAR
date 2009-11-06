//# ExampleEthernet.cc: a test program for the TH_Ethernet class
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

#if defined(HAVE_BGL) || defined(USE_NO_TH_ETHERNET)
// We can't use raw ethernet
int main (int argc, const char** argv)
{
  return 3;
}
#else

#include "DH_Ethernet.h"
#include <Transport/Connection.h>
#include <Transport/TH_Ethernet.h>
#include <Transport/TH_Mem.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/Timer.h>
#include <iostream>
#include <stdlib.h>

using namespace LOFAR;

#define MESSAGE_SIZE 1000

void sendData (DH_Ethernet& sender, Connection& con)
{
  for (int i=0;i<MESSAGE_SIZE;i++) {
    if (i <3 || i > MESSAGE_SIZE - 4) 
      sender.getBuffer()[i] = '*';
    else 
      sender.getBuffer()[i] = '-';
  }
  ASSERTSTR(con.write(), "Couldn't write");
  cout << "sent " << sender.getDataSize() << " bytes" << endl; 
}

void receiveData (DH_Ethernet& receiver, Connection& con)
{
  ASSERTSTR(con.read(), "could not read");
  cout << "Received " << receiver.getDataSize() << " bytes" << endl;
  const char* data2 = static_cast<char*>(receiver.getBuffer());

  for (int i=0;i<MESSAGE_SIZE;i++) {
    if (i <3 || i > MESSAGE_SIZE - 4) {
      ASSERTSTR(data2[i] == '*', "Received "<<data2[i]<<" instead of *");
    } else {
      ASSERTSTR(data2[i] == '-', "Received "<<data2[i]<<" instead of -");
    }
  }
}

void test (bool isReceiver, string interface, string remoteMac, string ownMac)
{
  DH_Ethernet DH_Sender("dh_sender", MESSAGE_SIZE);
  DH_Ethernet DH_Receiver("dh_receiver", MESSAGE_SIZE);
  TH_Ethernet proto(interface, remoteMac, ownMac, 0x000);
  proto.init();
  Connection conn("connection1", &DH_Sender, &DH_Receiver, &proto, true);

  if (isReceiver) { 
    DH_Receiver.init();
    receiveData (DH_Receiver, conn);
  }
  else { 
    DH_Sender.init();
    sendData(DH_Sender, conn);
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

#endif
