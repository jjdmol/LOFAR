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

#include <iostream>

#include <Transport/Transporter.h>
#include <Transport/TH_ShMem.h>
#include <DH_Example.h>

using namespace LOFAR;

int main(int argc, const char* argv[])
{
    
  cout << "libTransport ExampleShMem test program" << endl;

  TH_ShMem::init (argc, argv);
    
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
  Transporter& TR1 = DH1.getTransporter();
  Transporter& TR2 = DH2.getTransporter();
    
  // Assign an ID for each transporter by hand for now
  // This will be done by the framework later on
  TR1.setItsID(1);
  TR2.setItsID(2);

  TR1.setSourceAddr(&DH1);
  TR2.setSourceAddr(&DH2);
  //  TR2.setSourceAddr(15);
  
  // connect DH1 to DH2
  TR2.connectTo(&TR1, TH_ShMem::proto);
    
  // initialize the DataHolders
  DH1.init();
  DH2.init();
    
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
  DH1.write();
  DH2.read();
  // note that transport is bi-directional.
  // so this will also work:
  //   DH2.write();
  //   DH1.read();
  // 
  
  int sts=0;
  cout << "After transport  : " 
       << DH1.getBuffer()[0] << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBuffer()[0] << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBuffer()[0] != DH2.getBuffer()[0]
  ||  DH1.getCounter() != DH2.getCounter()) {
    cout << "Data in receiving DataHolder is incorrect" << endl;
    sts = 1;
  }
  TH_ShMem::finalize();
  return sts;
}
