//  CorbaTransportIn.h: Corba transport mechanism
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
//////////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <CEPFrame/BaseSim.h>
#include <Common/Debug.h>
#include <CEPFrame/Corba/CorbaTransportIn.h>

CorbaTransportIn::CorbaTransportIn(int aID)
{   
  bool bound = false;
  short attemptnr=0;
  while(!bound && attemptnr < 1000) {
    try{
      char name[80];
      
      // bind to the server for this client
      sprintf(name,"CorbaTransportOut_%i", aID);
      TRACER2("Try bind to server " << name);
      itsTransporterId = PortableServer::string_to_ObjectId(name);
      
      sprintf(name, "/CorbaTransportOut_%i", aID);
      itsTransporter   = CorbaTransportI::Transporter::_bind(name,
							     itsTransporterId);
      bound=true;
      TRACER2("Bound to server");
    } catch(const CORBA::Exception& e) {      
      usleep(100000);
      attemptnr++;
    }  
  }
  if (!bound) cerr << "ERROR: could not bind to Server " ;
  return;
}
 
CorbaTransportIn::~CorbaTransportIn()
{
  cout << "CorbaTranportIn destructor called." << endl;
}

bool CorbaTransportIn::getBuffer(void* bufferptr, unsigned long len) {
  TRACER2("make call to server ");
  try {

    // get the buffer from the server
    CorbaBufferType_var corbaBuffer = itsTransporter->transportBuffer();

    if (NULL == corbaBuffer)
    {
      cout << "no buffer available" << endl;
      return false;
    }

    // copy buffer contents to bufferptr
    memcpy(bufferptr, &corbaBuffer[0], len);

  } catch(const CORBA::Exception& e) {
    cerr << e << endl;
    return false;
  }  
  return true;
}

