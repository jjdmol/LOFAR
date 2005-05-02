//#  CorbaControlClient.cc: Corba control client
//#
//#  Copyright (C) 2000-2002
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/BaseSim.h>
#include <Common/Debug.h>
#include <CEPFrame/Corba/BS_Corba.h>
#include <unistd.h>

#include <CEPFrame/Corba/CorbaControl_c.hh>

main(int argc, char** argv) {
    AssertStr (argc == 3, 
	       "Need 2 arguments: SimulName and command(start/stop/abort)");
  try {
    // Start Orb Environment
    AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
    
    PortableServer::ObjectId_var     itsControllerId;
    CorbaControl::Controller_ptr itsController;
    char name[80];
    
    // bind to the server for this client
    sprintf(name,"CorbaController_%s",argv[1]);
    TRACER2("Try bind to server " << name);
    itsControllerId = PortableServer::string_to_ObjectId(name);
    
    sprintf(name, "/CorbaController_%s",argv[1]);
    itsController   = CorbaControl::Controller::_bind(name,
						      itsControllerId);
    TRACER2("Bound to server");
    
    TRACER2("make call to server ");
    if (strcmp(argv[2],"start") == 0)  itsController->start();
    if (strcmp(argv[2],"stop" ) == 0)  itsController->stop();
    if (strcmp(argv[2],"abort") == 0)  itsController->abort();

  } catch(const CORBA::Exception& e) {
    cerr << e << endl;
    return false;
  }  
  return true;
}

