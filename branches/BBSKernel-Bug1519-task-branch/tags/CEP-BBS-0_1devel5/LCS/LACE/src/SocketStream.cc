//#  SocketStream.cc: Abstract base class for several kinds of sockets
//#
//#  Copyright (C) 2008
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

//# Includes
#include <unistd.h>					// file stuff
#include <netinet/in.h>				// IPPROTO
//#include <netdb.h>					// fileStat
#include <Common/LofarLogger.h>
#include <LACE/SocketStream.h>

namespace LOFAR {
  namespace LACE {

//
// SocketStream()
//
SocketStream::SocketStream() :
	itsIsConnected(false)
{}


//
// ~SocketStream()
//
SocketStream::~SocketStream()
{
}


//
// read (buffer, nrBytes)
//
int SocketStream::read(void*	buffer, size_t	nrBytes)
{
	if (!itsIsConnected) {
		LOG_ERROR ("SocketStream in not yet connected");
		return (SK_NOT_OPENED);
	}
	return (SocketSAP::read(buffer, nrBytes));
}

//
// write (buffer, nrBytes)
//
// Returnvalue: < 0					error occured
//				>= 0 != nrBytes		intr. occured or socket is nonblocking
//				>= 0 == nrBytes		everthing went OK.
//
int SocketStream::write(const void*	buffer, size_t	nrBytes)
{
	if (!itsIsConnected) {
		LOG_ERROR ("SocketStream in not yet connected");
		return (SK_NOT_OPENED);
	}
	return (SocketSAP::write(buffer, nrBytes));
}


  } // namespace LACE
} // namespace LOFAR
