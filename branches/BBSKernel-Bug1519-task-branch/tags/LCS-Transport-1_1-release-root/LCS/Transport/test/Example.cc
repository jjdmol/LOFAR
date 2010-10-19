//# Example.cc: Test program for basic Transport classes
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

#include <Transport/TH_Mem.h>
#include <DH_Example.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main()
{
  INIT_LOGGER("Example.log_prop");
  cout << "Transport Example test program" << endl;
    
  DH_Example DH1("dh1", 1);
  DH_Example DH2("dh2", 1);
    
  // Assign an ID for each dataholder by hand for now
  // This will be done by the framework later on
  DH1.setID(1);
  DH2.setID(2);

  // connect DH1 to DH2 with non-blocking in-memory communication
  DH1.connectTo(DH2, TH_Mem(), false);
    
  // initialize
  DH1.init();
  DH2.init();
    
  // fill the DataHolders with some initial data
  DH1.getBufferElement(0) = fcomplex(17,-3.5);
  DH2.getBufferElement(0) = 0;
  DH1.setCounter(2);
  DH2.setCounter(0);
    
  cout << "Before transport : " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;
    
  // do the data transport
  DH1.write();
  DH2.read();
  
  cout << "After transport  : " 
       << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
       << " -- " 
       << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
       << endl;

  if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
  &&  DH1.getCounter() == DH2.getCounter()) {
    return 0;
  }
  cout << "Data in receiving DataHolder is incorrect" << endl;
  return 1;
}
