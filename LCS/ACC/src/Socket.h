//#  Socket.h: Implements a connection over a TCP/IP socket
//#
//#  Copyright (C) 2002-2004
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
//#	 This class implements a container of key-value pairs. The KV pairs can
//#  be read from a file or be merged with the values in another file.
//#  The class also support the creation of subsets.
//#
//#  $Id$

#ifndef ACC_SOCKET_H
#define ACC_SOCKET_H

#include <lofar_config.h>

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

//# Forward Declarations


// Description of class.
class Socket {
public:
	//# -------------- Construction and destruction ---------------
	Socket();
	~Socket();

	//# --------------- Copying is allowed --------------------
	Socket(const Socket& that);
	Socket& 	operator=(const Socket& that);

	//#-------------- Connection routines ---------------------
	// Tries to connect to the given host and service. The host parameter
	// may contain a hostname (resolved by DNS) or a dotted IP adres.
	bool	connect			(const string&	hostname, const int16	portnr);

	// Starts a listener-service on a TCP socket so a client can 
	// connect on it.
	bool	openListener	(const int16	portnr);

	// Wait for an incomming message at the given socket. The socket must
	// have been opened with Socket::listen before.
	// Returns the new socket to continue with, the user should check the
	// connectionstate of the returned socket because it is set to not-conn.
	// when a serious error occured.
	Socket	accept			();

	// Closes the connection in a nice way and releases the memory of the
	// connection.
	void	disconnect		();

	//# ---------------- Memory management for databuffers ----------------
	void	allocBuffer (int32		bufferSize);
	void	freeBuffer	();

	//# ---------------- Data exchange routines -------------------
	int32	send(const char*	message, const int32 messageLength);
	int32	poll(char 			**message, 
				 int32*			messageLength, 
				 const int32	timeout);

	//# ---------------- Accessor functions -----------------------
	inline int16	socketID() 	  const	{ return (itsSocketID);  	}
	inline bool		isConnected() const	{ return (itsConnected); 	}
	inline int16	bufferSize()  const	{ return (itsBufferSize);	}

private:
	string		itsHostname;
	int16		itsPortnr;
	int16		itsSocketID;
	int32		itsBufferSize;
	int32		itsRptr;
	int32		itsWptr;
	char*		itsData;
	bool		itsConnected;

	// move any data from the TCP/IP layer to our own buffers.
	int32	read();

	friend std::ostream& operator<<(std::ostream& os, const Socket &theSocket);
};

} // namespace LOFAR

#endif
