//  CorbaController.cc: Corba transport mechanism
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

///////////////////////////////////////////////////////////////////////////////////////

#include "CEPFrame/BaseSim.h"
#include "Common/Debug.h"
#include "CEPFrame/Corba/CorbaController.h"
#include "CEPFrame/VirtualMachine.h"
#include <unistd.h>

CorbaController::CorbaController(PortableServer::POA_var        aRootPOA,
				 PortableServer::POAManager_var aPOAManager,
				 const string&                  aControllername,
				 VirtualMachine*                aVM):
  itsVM(aVM)
{
  TRACER2("Entered CarboController C'tor");
  PortableServer::POA_var itsPOA;
  CORBA::PolicyList       policies;
  char                    name[80];

  policies.length(1);
  policies[(CORBA::ULong)0] =
    aRootPOA->create_lifespan_policy(PortableServer::PERSISTENT);

  // Create myPOA with the right policies
  sprintf(name, "CorbaController_%s", aControllername.c_str());
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
 
CorbaController::~CorbaController()
{
}

void CorbaController::start() {
  cout << "Called start" << endl;
  itsVM->trigger(VirtualMachine::start);
  return;
}

void CorbaController::stop() {
  TRACER2("Called stop");
  itsVM->trigger(VirtualMachine::stop);
  return;
}

void CorbaController::abort() {
  TRACER2("Called abort");
  itsVM->trigger(VirtualMachine::abort);
  return;
}

void CorbaController::connect(CORBA::Boolean _sourceside, 
			      CORBA::Long _channel,
			      CORBA::Long _ID) {
  TRACER2("Called connect " << _sourceside << " " << _channel << " " << _ID);
  TRACER2("... Currently not implemented");
  return;
}


