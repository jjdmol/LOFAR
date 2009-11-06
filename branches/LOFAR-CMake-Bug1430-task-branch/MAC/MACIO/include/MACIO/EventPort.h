//#  EventPort.h: LCS-Common-Socket based impl of a GCF TCPPort
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_MACIO_EVENTPORT_H
#define LOFAR_MACIO_EVENTPORT_H

// \file EventPort.h
// LCS-Common-Socket based impl of a GCF TCPPort

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Net/Socket.h>
#include <MACIO/GCF_Event.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// Define states for the connection fases.
enum {
	EP_CREATED = 0,
	EP_SEES_SB,
	EP_WAIT_FOR_SB_ANSWER,
	EP_KNOWS_DEST,
	EP_CONNECTING,
	EP_CONNECTED,
	EP_DISCONNECTED
};

// The EventPort class is a LCS/Common Socket based TCP port to make it
// possible for CEP applications to use the MAC protocols.
class EventPort
{
public:

	// EventPort (servicename, type, protocol, hostname)
	EventPort(const string&		aServiceMask,
						 bool				aServerSocket,
						 int				aProtocol,
						 const string&		aHostname = "",
						 bool				syncCommunication = false);

	// ~EventPort
	~EventPort();

	// connect()
	bool connect();

	// send(Event*)
	bool send(GCFEvent*	anEvent);

	// receive() : Event
	GCFEvent*	receive();

	// getStatus()
	int getStatus() { return (itsStatus); }

private:
	// static receiveEvent(aSocket)
	GCFEvent*	receiveEvent(Socket*	aSocket);

	// static sendEvent(Socket*, Event*)
	void sendEvent(Socket*		aSocket,
						  GCFEvent*		anEvent);

	// _internal routines: see source code for description
	string	_makeServiceName(const string&	aServiceMask, int32		aNumber);
	bool	_setupConnection();
	int32	_askBrokerThePortNumber();
	int32	_waitForSBAnswer();
	int32	_startConnectionToPeer();
	int32	_waitForPeerResponse();
	void	_peerClosedConnection();

	EventPort();

	// Copying is not allowed
	EventPort(const EventPort&	that);
	EventPort& operator=(const EventPort& that);

	//# --- Datamembers ---
	int32			itsPort;
	string			itsHost;
	string			itsServiceName;
	Socket*			itsSocket;
	Socket*			itsListenSocket;
	Socket*			itsBrokerSocket;
	int32			itsStatus;
	bool			itsSyncComm;
	bool			itsIsServer;
};


// @}
  } // namespace MACIO
} // namespace LOFAR

#endif
