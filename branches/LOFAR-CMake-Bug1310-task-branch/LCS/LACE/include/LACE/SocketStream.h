//#  SocketStream.h: Abstract base class for all kind of different sockets.
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

#ifndef LOFAR_LACE_SOCKETSTREAM_H
#define LOFAR_LACE_SOCKETSTREAM_H

// \file SocketStream.h
// Abstract base class for all kind of different sockets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <LACE/SocketSAP.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// ...
class SocketStream : public SocketSAP
{
public:
	SocketStream();
	virtual ~SocketStream();

	int	 read (void*		buffer, size_t	nrBytes);
	int	 write(const void*	buffer, size_t	nrBytes);
	
	void close() { SocketSAP::close(); }
	bool isConnected()	{ return(itsIsConnected); }

protected:
	friend class SocketConnector;
	friend class SocketAcceptor;
	int  open (const InetAddress&	anAddress, bool reuseAddress = true)
		{ return (doOpen(anAddress, reuseAddress)); }

	
private:
	//# --- Datamembers ---
	bool		itsIsConnected;		
};


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
