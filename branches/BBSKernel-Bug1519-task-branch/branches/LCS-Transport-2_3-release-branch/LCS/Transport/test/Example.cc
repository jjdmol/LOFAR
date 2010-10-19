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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <iostream>

#include <Transport/TH_Mem.h>
#include <DH_Example.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main()
{
  INIT_LOGGER("Example.log_prop");
  cout << "Transport Example test program" << endl;
  bool result = false;
  try {
    DH_Example DH1("dh1", 1);
    DH_Example DH2("dh2", 1);
    
    // connect DH1 to DH2 with non-blocking in-memory communication
    TH_Mem memTH;
    Connection conn("connection1", &DH1, &DH2, &memTH, false);
    
    // initialize
    DH1.init();
    DH2.init();

    // fill the DataHolders with some initial data
    DH1.getBufferElement(0) = makefcomplex(17,-3.5);
    DH2.getBufferElement(0) = makefcomplex(0,0);
    DH1.setCounter(2);
    DH2.setCounter(0);
    
    cout << "Before transport : " 
	 << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
	 << " -- " 
	 << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
	 << endl;
    
    // do the data transport
    ASSERT(conn.write() == Connection::Finished);
    ASSERT(conn.read() == Connection::Finished);
    
    cout << "After transport  : " 
	 << DH1.getBufferElement(0) << ' ' << DH1.getCounter()
	 << " -- " 
	 << DH2.getBufferElement(0) << ' ' << DH2.getCounter()
	 << endl;
    
    if (DH1.getBufferElement(0) == DH2.getBufferElement(0)
	&&  DH1.getCounter() == DH2.getCounter()) {
      result = true;
    }
    else {
    cout << "Data in receiving DataHolder is incorrect" << endl;
    result = false;
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    result = false;
  }
  return (result ? 0:1);
}
