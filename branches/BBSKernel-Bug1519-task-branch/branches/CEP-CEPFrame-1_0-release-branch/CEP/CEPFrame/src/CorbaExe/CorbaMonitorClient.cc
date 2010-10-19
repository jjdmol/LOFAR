//  CorbaMonitorClient.cc: Corba monitor client
//
//  Copyright (C) 2000-2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
////////////////////////////////////////////////////////////////////////


#include "CEPFrame/BaseSim.h"
#include "Common/Debug.h"
#include "CEPFrame/Corba/BS_Corba.h"
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/Corba/CorbaControl_c.hh"

main(int argc, char** argv) {
    AssertStr (argc >= 3, 
	       "Need 2 arguments: SimulName and command(getValue) and possibly arguments");
  try {
    // Start Orb Environment
    AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
    
    PortableServer::ObjectId_var     itsMonitorId;
    CorbaControl::Monitor_ptr itsMonitor;
    char name[80];
    
    // bind to the server for this client
    sprintf(name,"CorbaMonitor_%s",argv[1]);
    TRACER2("Try bind to server " << name);
    itsMonitorId = PortableServer::string_to_ObjectId(name);
    
    sprintf(name, "/CorbaMonitor_%s",argv[1]);
    itsMonitor   = CorbaControl::Monitor::_bind(name,
						      itsMonitorId);
    TRACER2("Bound to server");
    
    TRACER2("make call to server ");
    if (strcmp(argv[2],"getValue") == 0)  
      cout << itsMonitor->getValue(argv[3]) << endl;

    if (strcmp(argv[2],"scan") == 0)
      for (int i=0; i<10; i++) {
	cout << i << " "  
	     << itsMonitor->getValue("size") << " " 
	     << itsMonitor->getValue("perf") 
	     << endl;
	usleep(100);
      }

  } catch(const CORBA::Exception& e) {
    cerr << e << endl;
    return false;
  }  
  return true;
}

