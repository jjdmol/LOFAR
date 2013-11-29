//  GPI_TH_Port.cc: POSIX Socket based Transport Holder
//
//  Copyright (C) 2000-2004
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

#include <lofar_config.h>

#include <GPI_TH_Port.h>
#include <Transport/DataHolder.h>
#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {
GPITH_Port::~GPITH_Port()
{
	LOG_TRACE_OBJ("~GPITH_Port");
}

GPITH_Port* GPITH_Port::make() const
{
	LOG_TRACE_OBJ(formatString("make GPITH_Port(port=\"%s\")", 
								_port.getName().c_str()));

  return (new GPITH_Port(_port));
}
  
bool GPITH_Port::init()
{
  return _port.isConnected();
}

string GPITH_Port::getType() const
{
	return ("GPITH_Port");
}
  
int32 GPITH_Port::recvNonBlocking(void*	buf, int32	nrBytes, int32 /*tag*/, int offset, LOFAR::DataHolder*)
{
  // Note: buf, offset and nrBytes are outerWorld view on databuffer contents
  // itsReadOffset is innerWorld view on databuffer contents

  LOG_TRACE_OBJ(formatString(
      "TH_Socket::recvNonBlocking(%d), offset=%d", 
      nrBytes, offset));

  if (itsReadOffset >= offset+nrBytes) // already in buffer?
  {    
    itsReadOffset = 0;            // reset internal offset
    itsLastCmd = CmdNone;         // async admin.
    return (nrBytes);
  }

  if (itsLastCmd == CmdNone) // adopt offset on fresh entry
  {        
    itsReadOffset = offset;
  }
  itsLastCmd  = CmdRecvNonBlock;        // remember where to wait for.

  if (!init()) // be sure we are connected
  {                
    return (0);
  }

  // read remaining bytes
  int32 bytesRead = _port.recv((char*)buf - offset + itsReadOffset, 
                               (offset + nrBytes) - itsReadOffset);
  LOG_TRACE_VAR_STR("Read " << bytesRead << " bytes from socket");

  // Errors are reported with negative numbers
  if (bytesRead < 0) // serious error?
  {            
    // It's a total mess, anything could have happend. Bail out.
    LOG_DEBUG_STR("TH_Socket: serious read-error, result=" << bytesRead);
    itsLastCmd    = CmdNone;
    itsReadOffset = 0;            // it's a total mess
    return (false);
  }

  // Some data was read
  if (bytesRead+itsReadOffset < offset+nrBytes) // everthing read?
  { 
    itsReadOffset += bytesRead;       // No, update readoffset.
    return(0);
  }

  itsReadOffset = 0;              // message complete, reset
  itsLastCmd    = CmdNone;          // async admin.

  return (nrBytes);
}

//
// sendNonBlocking(buf, nrBytes, tag)
//
bool GPITH_Port::sendNonBlocking(void* buf, int32 nrBytes, int32 /*tag*/, DataHolder* /*dh*/)
{
  if (!init()) 
  {
    return (false);
  }
  
  GPIBlobEvent e;
  e.blobData = buf;
  e.blobSize = nrBytes;

    
	// Now we should have a connection
	int32 sent_len = _port.send(e);

	return (sent_len == nrBytes);
}
         
void* GPITH_Port::GPIBlobEvent::pack(unsigned int& packsize)
{
  resizeBuf(blobSize);
  
  memcpy(_buffer, blobData, blobSize);

  packsize = blobSize;
  return _buffer;
}

bool GPITH_Port::recvBlocking (void*, int, int, int, LOFAR::DataHolder*)
{
  THROW(LOFAR::Exception, "No recvBlocking method implemented in this TransportHolder: " 
        + getType());
  return false;
}

bool GPITH_Port::sendBlocking (void*, int, int, LOFAR::DataHolder*)
{
  THROW(LOFAR::Exception, "No sendBlocking method implemented in this TransportHolder: "
        + getType());
 return false;
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
