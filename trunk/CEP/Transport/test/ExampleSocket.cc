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
#include <Transport/Transporter.h>
#include <Transport/TH_Socket.h>
#include <DH_Example.h>

using namespace LOFAR;

int main (int argv, char** argc) {

  cout << "Transport example program using TH_Socket" << endl;

  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);

  Transporter& TR1 = DH1.getTransporter();
  Transporter& TR2 = DH2.getTransporter();

  TR1.setItsID(1);
  TR2.setItsID(2);

  TR1.setIsBlocking(true);
  TR2.setIsBlocking(true);

  TH_Socket proto("localhost", "localhost", 8923);

  TR1.connectTo(&TR2, proto);

  TR1.init();
  TR2.init();

 // fill the DataHolders with some initial data
  DH1.getBuffer()[0] = fcomplex(17,-3.5);
  DH2.getBuffer()[0] = 0;
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  DH2.read();
  DH1.write();
  // note that transport is bi-directional.
  // so this will also work:
  //   DH2.write();
  //   DH1.read();
  // 
  
  cout << "After transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] == DH2.getBuffer()[0]
  &&  DH1.getCounter() == DH2.getCounter()) {
    return 0;
  }
  cout << "Data in receiving DataHolder is incorrect" << endl;
  return 1;
}
