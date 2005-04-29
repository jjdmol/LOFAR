//  CorbaTransportOut.cc: Corba transport mechanism
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
///////////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/BaseSim.h>
#include <Common/Debug.h>
#include <CEPFrame/Corba/CorbaTransportOut.h>

CorbaTransportOut::CorbaTransportOut(
			     PortableServer::POA_var        itsRootPOA,
			     PortableServer::POAManager_var itsPOAManager,
			     int aID)
{
  PortableServer::POA_var itsPOA;
  CORBA::PolicyList       policies;
  char                    name[80];

  itsBuffers = new MTCircularBuffer<CorbaBufferType*>(5);
  itsBuffer = (CorbaBufferType*)NULL;

  policies.length(1);
  policies[(CORBA::ULong)0] =
    itsRootPOA->create_lifespan_policy(PortableServer::PERSISTENT);

  // Create myPOA with the right policies
  sprintf(name, "CorbaTransportOut_%i", aID);
  itsPOA = itsRootPOA->create_POA(name, 
				  itsPOAManager,
				  policies); 

  PortableServer::ObjectId_var transporterId  =
    PortableServer::string_to_ObjectId(name);
  itsPOA->activate_object_with_id(transporterId, this);

  TRACER2( "Created " << name);

  itsPOAManager->activate();
  TRACER2("POA activated.");
}
 
CorbaTransportOut::~CorbaTransportOut()
{
  delete itsBuffers;
  delete itsBuffer;
}

CorbaBufferType* CorbaTransportOut::transportBuffer()
{
  CorbaBufferType* theBuffer = NULL;

  TRACER2("transportBuffer called in the server.");

  while (false == itsBuffers->Get(&theBuffer))
  {
    usleep(10000);
  }
  return theBuffer;
}

bool CorbaTransportOut::putBuffer(void* bufferptr, unsigned long len)
{
  try 
  {
    //    cout << "CorbaTransportOut::putBuffer: len = " << len << endl;
#if 0
    char* buf = new char[len];
    memcpy(buf, bufferptr, len);
    itsBuffer = new CorbaBufferType(0, len, (CORBA::Char*)buf, 1);
#else
    // copy only the pointer; data is NOT copied!
    itsBuffer = new CorbaBufferType(0, len, (CORBA::Char*)bufferptr, 0);
#endif    

    while (false == itsBuffers->Put(itsBuffer))
    {
      usleep(10000);
    }
  }
  catch (const CORBA::Exception& e)
  {
    cerr << e << endl;
    return false;
  }

  return true;
}
