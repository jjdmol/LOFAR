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

#include <GPI_TH_Port.h>
#include <Transport/Transporter.h>
#include <Transport/DataHolder.h>
#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR
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
  
string GPITH_Port::getType() const
{
	return ("GPITH_Port");
}
  
bool GPITH_Port::connectionPossible (int32 srcRank, int32 dstRank) const
{
	return (srcRank == dstRank);
}
  
bool GPITH_Port::recvNonBlocking(void*	buf, int32	nrBytes, int32 /*tag*/)
{
	if (!_port.isConnected()) 
  {
		return (false);
	}

	// Now we should have a connection
	if (_port.recv (buf, nrBytes) != nrBytes) 
  {
		THROW(AssertError, "GPITH_Port: data not succesfully received");
	}
	return (true);
}

bool GPITH_Port::recvVarNonBlocking(int32	tag)
{
  if (!_port.isConnected()) 
  {
		return (false);
	}

	// Read the blob header.
	DataHolder* target = getTransporter()->getDataHolder();
	void*	buf   = target->getDataPtr();
	int32	hdrsz = target->getHeaderSize();

	if (!recvNonBlocking(buf, hdrsz, tag)) {
		return (false);
	}

	// Extract the length and resize the buffer.
	int32 size = DataHolder::getDataLength (buf);
	target->resizeBuffer (size);
	buf = target->getDataPtr();
	// Read the remainder.
	bool result = recvNonBlocking (static_cast<char*>(buf)+hdrsz, size-hdrsz, tag);

  return (result);
}

//
// sendNonBlocking(buf, nrBytes, tag)
//
bool GPITH_Port::sendNonBlocking(void*	buf, int32	nrBytes, int32	 /*tag*/)
{
  if (!_port.isConnected()) 
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
  
  memcpy(_buffer, &blobData, blobSize);

  packsize = blobSize;
  return _buffer;
}


} // namespace LOFAR
