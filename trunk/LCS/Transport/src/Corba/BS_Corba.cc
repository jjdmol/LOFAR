//  BS_Corba.cc: Corba transport mechanism
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
///////////////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/Corba/BS_Corba.h>
#include <Common/Debug.h>
#include <unistd.h>

pthread_t                      BS_Corba::itsORBThread;
CORBA::ORB_var                 BS_Corba::itsORB=0;
PortableServer::POA_var        BS_Corba::itsRootPOA=0;
PortableServer::POAManager_var BS_Corba::itsPOAManager=0;


BS_Corba::BS_Corba()
{}


BS_Corba::~BS_Corba ()
{
#if 0
  if (itsORB) {
    itsORB->shutdown();
    (void)pthread_join(TH_Corba::itsORBThread, NULL);
  }
#endif
}

bool BS_Corba::init()
{
  int    argc=0;
  char **argv=0;
  
  TRACER2("Connect to orb and get root POA manager");
  
  try {
    TRACER2("Initialize the ORB.");
    itsORB = CORBA::ORB_init(argc, argv);
    
    TRACER2("get a reference to the root POA");
    CORBA::Object_var obj = itsORB->resolve_initial_references("RootPOA");
    itsRootPOA = PortableServer::POA::_narrow(obj);
    
    TRACER2("get and activate the POA manager");
    itsPOAManager = itsRootPOA->the_POAManager();
  
  }  catch (const CORBA::Exception& e)  {
    cerr << e << endl;
    return false;
  }
  
  TRACER2("ORB initialised and root POA manager activated.");
  
  TRACER2("start orb in a separate thread");
  pthread_create(&itsORBThread, 
		 NULL,
		 &BS_Corba::run_orb, NULL);
  
  return true;
}


void* BS_Corba::run_orb(void *)
{
  try
  {
    TRACER2("Run ORB");
    itsORB->run();
  }
  catch (const CORBA::Exception& e)
    {
      cerr << e << endl;
    return NULL;
    }

  return NULL;
} 

