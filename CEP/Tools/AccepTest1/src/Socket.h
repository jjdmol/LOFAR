//#  Socket.h:  Creates an object used to access the sys/socket
//#             system calls. Extra checks and administration is 
//#             added to these access methods for  ease of use.
//#             Implements a connection over a TCP/IP socket
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

#ifndef CEPF_SOCKET_H
#define CEPF_SOCKET_H

/* This Socket class has been included in CEPFrame temporarily. It is a
   copy of LCS/ACC/Socket. Currently LCS/ACC/Socket does not compile due
   to the fact that log4cplus does not work. Until then, the class below
   is used.
*/

//#include <lofar_config.h>

#include <string>
using std::string;

//# Includes
//#include <Common/LofarTypes.h>
//#include <Common/lofar_string.h>

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
	bool	connect			(const string&	hostname, const short	portnr);

	// Starts a listener-service on a TCP socket so a client can 
	// connect on it.
	bool	openListener	(const short	portnr);

	// Wait for an incomming message at the given socket. The socket must
	// have been opened with Socket::listen before.
	// Returns the new socket to continue with, the user should check the
	// connectionstate of the returned socket because it is set to not-conn.
	// when a serious error occured.
	Socket	accept();	

	// set the data buffer used in recv calls.
	// the default values will result in a newly allocated buffer with the default size
	// When an external bufferptr is passed, that buffer will be used
	void setBuffer(int   buffersize=4096,      // specify receive buffer size
			 char* bufferptr =NULL);     // address of exteral receive buffer

	// Closes the connection in a nice way and releases the memory of the
	// connection.
	void	disconnect		();
	void	shutdown		();

	//# ---------------- Memory management for databuffers ----------------
	void	allocBuffer (int		bufferSize);
	void	freeBuffer	();

	//# ---------------- Data exchange routines -------------------
	int	send(const char* message, 
		     const int messageLength);
	int	recv(char* buf=NULL,
		     const int   len=0);
	int	poll(char **message, 
		     int*  messageLength, 
		     const int timeout);

	//# ---------------- Accessor functions -----------------------
	inline short	socketID()    const	{ return (itsSocketID);  	}
	inline bool	isConnected() const	{ return (itsConnected); 	}
	inline short	bufferSize()  const	{ return (itsBufferSize);	}

	typedef enum {
		SK_OK         =  0,		// Ok
		SOCKET        = -1,		// Can't create socket
		BIND          = -2,		// Can't bind local address
		CONNECT       = -3,		// Can't connect to server
		ACCEPT        = -4,		// Can't accept client socket
		BADHOST       = -5,		// Bad server host name given
		BADADDRTYPE   = -6,		// Bad address type
		READERR       = -7,		// Read error
		WRITERR       = -8,		// Write error
		PEERCLOSED    = -9,		// Remote client closed connection
		INCOMPLETE    = -10,	// Couldn't read/write whole message
		INVOP         = -11,	// Invalid operation
		SOCKOPT       = -12,	// sockopt() failure
		PORT          = -13,	// wrong port/service specified
		PROTOCOL      = -14,	// invalid protocol
		LISTEN        = -15,	// listen() error
		TIMEOUT       = -16,	// timeout
		INPROGRESS    = -17,	// connect() in progress
		NOMORECLI     = -18,	// No more clients
		SHUTDOWN      = -19,	// shutdown() failure
		NOINIT        = -20		// uninitialized socket
	} ErrorCodes;

private:
   string		itsHostname;
   short		itsPortnr;
   short		itsSocketID;
   int		itsBufferSize;
   int		itsRptr;
   int		itsWptr;
   char*		itsData;
   bool		        itsConnected;
   bool                 itsIsMyBuffer;
   bool                 itsIsAllocated;
   
	friend std::ostream& operator<<(std::ostream& os, const Socket &theSocket);
};

} // namespace LOFAR

#endif
