//  CorbaMonitor.cc: Corba monitor mechanism
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
//
/////////////////////////////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/BaseSim.h>
#include <Common/Debug.h>
#include <CEPFrame/Corba/CorbaMonitor.h>
#include <CEPFrame/WorkHolder.h>

CorbaMonitor::CorbaMonitor(PortableServer::POA_var        aRootPOA,
			   PortableServer::POAManager_var aPOAManager,
			   const string&                        aMonitorname,
			   WorkHolder*                    aWorkHolder):
  itsWorker(aWorkHolder)
{
  TRACER2("Entered CorbaMonitor C'tor");
  PortableServer::POA_var itsPOA;
  CORBA::PolicyList       policies;
  char                    name[80];

  policies.length(1);
  policies[(CORBA::ULong)0] =
    aRootPOA->create_lifespan_policy(PortableServer::PERSISTENT);

  // Create myPOA with the right policies
  sprintf(name, "CorbaMonitor_%s", aMonitorname.c_str());
  itsPOA = aRootPOA->create_POA(name, 
				aPOAManager,
				policies); 

  PortableServer::ObjectId_var transporterId  =
    PortableServer::string_to_ObjectId(name);
  itsPOA->activate_object_with_id(transporterId, this);
  
  TRACER2( "Created " << name);
  
  aPOAManager->activate();
  TRACER2("POA activated.");
}
 
CorbaMonitor::~CorbaMonitor()
{
}

CORBA::Long CorbaMonitor::getValue(const char* _name) {
  TRACER2("Called CorbaMonitor::getValue " << _name);
  cout << "getValue" << endl;
  int result = 0;
  if (strcmp(_name,"dump") == 0) {
    itsWorker->dump();  
  } else {
    result = itsWorker->getMonitorValue(_name);
  }

  return (CORBA::Long) result;
}

