//#  EventPort.cc: (raw) socket based implementation to exchange Events
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/EventPort.h>
#include <SB_Protocol.ph>
//#include "GSB_Defines.h"

namespace LOFAR {
  namespace MACIO {

// Note: the difference with GCF-ports is that this port is only based on the
// LCS/Common sockets and does therefor not depend on GCF_Tasks.

//
// EventPort (name, type, protocol)
//
EventPort::EventPort(const string&		aServiceMask,
					 bool				aServerSocket,
					 int				aProtocol,
					 const string&		aHostname,
					 bool				syncCommunication) :
	itsPort			(0),
	itsHost			(aHostname),
	itsServiceName	(_makeServiceName(aServiceMask, 0)),
	itsSocket		(0),
	itsListenSocket	(0),
	itsBrokerSocket	(0),
	itsStatus		(EP_CREATED),
	itsSyncComm		(syncCommunication),
	itsIsServer		(aServerSocket),
	itsBtsToRead	(0),
	itsTotalBtsRead	(0),
	itsReadState	(0),
	itsEventHdr		(0),
	itsEventBuf		(0)
{
    (void)aProtocol; // avoid compiler warning

	if (itsHost.empty() || itsHost == "localhost") {
		itsHost = myHostname(false);
	} 

	// We always need a socket to the serviceBroker for getting a portnumber we may use.
	itsBrokerSocket = new Socket("ServiceBroker", itsHost, toString(MAC_SERVICEBROKER_PORT));
	ASSERTSTR(itsBrokerSocket, "can't allocate socket to serviceBroker");

	_setupConnection();
}

//
// ~EventPort
//
EventPort::~EventPort()
{
	if (itsSocket) {
		itsSocket->shutdown();
		delete itsSocket;
	};

	if (itsListenSocket) {
		itsListenSocket->shutdown();
		delete itsListenSocket;
	};

	if (itsBrokerSocket) {
		itsBrokerSocket->shutdown();
		delete itsBrokerSocket;
	};

	if (itsEventHdr) delete itsEventHdr;
}

//
// connect()
//
bool EventPort::connect()
{
	return (_setupConnection());
}

//
// _setupConnection()
//
// Depending of the stage that is already reached in the connection process, this
// function tries to continue from there till a connection is made.
//
// Note: all error handling is in this 'overall' routine to keep the subroutines
//		 that are called as easy as possible.
//
bool EventPort::_setupConnection()
{
	// test most likely state first for performance
	if (itsStatus == EP_CONNECTED) {
		return (true);
	}

	// not connected, so we left somewhere in the connection sequence, pick the sequence
	// at the right place.

	// No connection to the SB yet?
	if (itsStatus == EP_CREATED) {
		LOG_INFO ("Trying to make connection with the ServiceBroker");
		itsBrokerSocket->connect(itsSyncComm ? -1 : 500);	// try to connect, wait forever or 0.5 sec
		if (!itsBrokerSocket->isConnected()) {	// failed?
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot connect to the ServiceBroker");
			}
			return (false);		// async socket allows failures.
		}
		itsStatus = EP_SEES_SB;
		itsBrokerSocket->setBlocking(itsSyncComm);		// no other tasks, do rest blocking
	}

	// second step: ask service broker the portnumber
	if (itsStatus == EP_SEES_SB) {
		itsStatus += _askBrokerThePortNumber();
		if (itsStatus <= EP_SEES_SB) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the ServiceBroker");
			}
			return (false);		// async socket allows failures.
		}
	}

	// third step: wait for the answer of the SB
	if (itsStatus == EP_WAIT_FOR_SB_ANSWER) {
		itsStatus += _waitForSBAnswer();
		if (itsStatus <= EP_WAIT_FOR_SB_ANSWER) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
	}

	// fourth step: try to connect to the real socket.
	if (itsStatus == EP_KNOWS_DEST) {
		itsStatus += _startConnectionToPeer();
		if (itsStatus <= EP_KNOWS_DEST) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
	}

	// fifth step: wait for response of connection request
	if (itsStatus == EP_CONNECTING) {
		itsStatus += _waitForPeerResponse();
		if (itsStatus <= EP_CONNECTING) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
	}

	return (itsStatus == EP_CONNECTED);
}

//
// send a message to the message broker requesting a portnumber
//
int32 EventPort::_askBrokerThePortNumber()
{
	// construct the question.
	if (itsIsServer) {
		// tell SB the name of our service
		SBRegisterServiceEvent	request;
		request.seqnr		= 5;	// or any other number
		request.servicename	= itsServiceName;

		// send question
		sendEvent(itsBrokerSocket, &request);
	}
	else {	// client socket
		// ask SB the portnumber of the (operational) service.
		SBGetServiceinfoEvent	request;
		request.seqnr		= 5;	// or any other number
		request.servicename	= itsServiceName;
		request.hostname	= itsHost;

		// send question
		sendEvent(itsBrokerSocket, &request);
	}

	return (1);		// goto next state
}

//
// waitForSBAnswer()
//
// Wait until we received a response from the service broker.
//
int32 EventPort::_waitForSBAnswer()
{
	// wait for response
	GCFEvent*	answerEventPtr(receiveEvent(itsBrokerSocket));
	if (!answerEventPtr) {
		return (0);		// stay in this mode
	}

	// a complete event was received, handle it.
	if (itsIsServer) {
		SBServiceRegisteredEvent response(*answerEventPtr);
		if (response.result != 0) {
			LOG_ERROR_STR("Service " << itsServiceName << " is already on the air.");
			return (-1);	// next time ask again
		}
		itsPort = response.portnumber;
		LOG_INFO_STR("Service " << itsServiceName << " will be at port " << itsPort);
		// note: keep connection with Broker so he knows we are on the air.
	}
	else {	// client socket
		SBServiceInfoEvent response(*answerEventPtr);
		if (response.result != 0) {
			LOG_ERROR_STR("Service " << itsServiceName << " is unknown");
			return (-1);	// next time ask again
		}
		itsPort = response.portnumber;
		LOG_INFO_STR("Service " << itsServiceName << " is at port " << itsPort);

		// close connection with Broker.
		itsBrokerSocket->shutdown();
		itsBrokerSocket = 0;
	}

	return (1);		// continue with next stage
}

//
// _startConnectionToPeer()
//
int32 EventPort::_startConnectionToPeer()
{
	// Finally we can make the real connection
	if (itsIsServer) {
		LOG_INFO_STR ("Opening listener on port " << itsPort);
		itsListenSocket = new Socket(itsServiceName, toString(itsPort), Socket::TCP);
		itsSocket = itsListenSocket->accept(itsSyncComm ? -1 : 500);
	}
	else {
		LOG_INFO_STR ("Trying to make connection with " << itsServiceName);
		itsSocket = new Socket(itsServiceName, itsHost, toString(itsPort), Socket::TCP);
		itsSocket->connect(itsSyncComm ? -1 : 500);	// try to connect, wait forever or 0.5 sec
	}
	return (1);
}

//
// _waitForPeerResponse()
//
int32 EventPort::_waitForPeerResponse()
{
	// connection (already) successful?
	if (itsSocket->isConnected()) {
		itsSocket->setBlocking(itsSyncComm);
		return (1);
	}

	// do a retry
	if (itsIsServer) {
		itsSocket = itsListenSocket->accept(itsSyncComm ? -1 : 500);
	}
	else {
		itsSocket->connect(itsSyncComm ? -1 : 500);	// try to connect, wait forever or 0.5 sec
	}

	return (itsSocket->isConnected());
}

//
// _peerClosedConnection()
//
void EventPort::_peerClosedConnection()
{
	// yeah.... what is wise to do here????
	ASSERTSTR(false, "Other side closed the connection, bailing out");

}

//
// send(Event*)
//
bool EventPort::send(GCFEvent*	anEvent)
{
	if (!_setupConnection()) {
		return (false);
	}

	sendEvent(itsSocket, anEvent);
	return (true);
}

//
// receive() : Event
//
GCFEvent*	EventPort::receive()
{
	return (_setupConnection() ? receiveEvent(itsSocket) : 0L);
}

//
// _makeServiceName(mask, number)
//
string	EventPort::_makeServiceName(const string&	aServiceMask, int32		aNumber)
{
	// NOTE: this code is copied from GCF/TM/Port/GCF_PortInterface.cc
	string	instanceNrStr;
	if (aNumber) {
		instanceNrStr = toString(aNumber);
	}

	return(formatString(aServiceMask.c_str(), instanceNrStr.c_str()));
}

// -------------------- STATIC FUNCTIONS --------------------
//
// sendEvent(Socket*, Event*)
//
void EventPort::sendEvent(Socket*		aSocket,
						  GCFEvent*		anEvent)
{
	// Serialize the message and write buffer to port
	anEvent->pack();
	int32 	btsWritten = aSocket->write(anEvent->packedBuffer(), anEvent->bufferSize());
	ASSERTSTR(btsWritten == (int32)anEvent->bufferSize(), 
			  "Only " << btsWritten << " of " << anEvent->bufferSize() << " bytes written");
}

//
// static receiveEvent(aSocket)
//
GCFEvent*	EventPort::receiveEvent(Socket*	aSocket)
{
	// make sure we have room for the header
	if (!itsEventHdr) {
		itsEventHdr = new GCFEvent;
	}
	int32			btsRead(0);

	// first read signal (= eventtype) field
	if (itsReadState == 0) {
		// cleanup old garbage if any
		if (itsEventBuf) {
			delete itsEventBuf;
			itsEventBuf = 0;
		}
		btsRead = aSocket->read((void*) &(itsEventHdr->signal), sizeof(itsEventHdr->signal));
		if (btsRead < 0) {
			_peerClosedConnection();
			return (0);
		}
		if (btsRead != sizeof(itsEventHdr->signal)) {
			if (aSocket->isBlocking()) {
				ASSERTSTR(false, "Event-type was not received");
			}
			return (0);		// async socket allows failures.
		}
		itsReadState++;
	}

	// next read the length of the rest of the message
	if (itsReadState == 1) {
		btsRead = aSocket->read((void*) &(itsEventHdr->length), sizeof(itsEventHdr->length));
		if (btsRead < 0) {
			_peerClosedConnection();
			return (0);
		}
		if (btsRead != sizeof(itsEventHdr->length)) {
			if (aSocket->isBlocking()) {
				ASSERTSTR(false, "Event-length was not received");
			}
			return (0);		// async socket allows failures.
		}
		itsReadState++;
		itsBtsToRead = itsEventHdr->length;		// get size of data part

		// When there is addional info (which is normally the case) allocate a
		// larger buffer to store a complete packed event
		if (itsBtsToRead > 0) {
			itsEventBuf = new char[GCFEvent::sizePackedGCFEvent + itsEventHdr->length];
			ASSERTSTR(itsEventBuf, "Could not allocate buffer for " << 
						GCFEvent::sizePackedGCFEvent + itsEventHdr->length << " bytes");
			memcpy(itsEventBuf,					     &itsEventHdr->signal, GCFEvent::sizeSignal);
			memcpy(itsEventBuf+GCFEvent::sizeSignal, &itsEventHdr->length, GCFEvent::sizeLength);
		}
	}

	// finally read the datapart of the event
	if (itsReadState == 2) {
		LOG_DEBUG_STR("Still " << itsBtsToRead << " bytes of data to get");

		btsRead = 0;
		if (itsBtsToRead) {
			btsRead = aSocket->read(itsEventBuf + GCFEvent::sizePackedGCFEvent + itsTotalBtsRead, itsBtsToRead);
			if (btsRead < 0) {
				_peerClosedConnection();
				return (0);
			}
			if (btsRead != itsBtsToRead) {
				if (aSocket->isBlocking()) {
					ASSERTSTR(false, "Only " << btsRead << " bytes of msg read: " << itsBtsToRead);
				}
				return (0);		// async socket allow failures
			}
		}

		if (btsRead == itsBtsToRead) {	// everything received?
			// reset own admin
			itsReadState    = 0;
			itsTotalBtsRead = 0;
			itsBtsToRead    = 0;
//			{ string s; hexdump(s, itsEventBuf, GCFEvent::sizePackedGCFEvent + itsEventHdr->length); LOG_DEBUG(s); }
			itsEventHdr->_buffer = itsEventBuf; // attach buffer to event
			return (itsEventHdr);
		}
	}

	// not all information received yet
	return (0L);
}

 
  } // namespace MACIO
} // namespace LOFAR
