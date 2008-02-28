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
#include <Common/hexdump.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/TM/GCF_Event.h>
#include <GCF/TM/EventPort.h>
#include <SB_Protocol.ph>
#include "GSB_Defines.h"

namespace LOFAR {
  namespace GCF {
    namespace TM {

// Note: the difference with GCF-ports is that this port is only based on the
// LCS/Common sockets and does therefor not depend on GCF_Tasks.

static char			receiveBuffer[24*4096];

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
	itsIsServer		(aServerSocket)
{
	if (itsHost.empty() || itsHost == "localhost") {
		itsHost = Common::myHostname(false);
	} 

	// We always need a socket to the serviceBroker for getting a portnumber we may use.
	itsBrokerSocket = new Socket("ServiceBroker", itsHost, toString(MAC_SERVICEBROKER_PORT));
	ASSERTSTR(itsBrokerSocket, "can't allocate socket to serviceBroker");
cout << "BS0=" << itsBrokerSocket << endl;

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
}

//
// connect()
//
bool EventPort::connect()
{
cout << "BS1=" << itsBrokerSocket << endl;
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
		LOG_DEBUG ("Trying to make connection with the ServiceBroker");
cout << "BS2=" << itsBrokerSocket << endl;
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
		if (!_askBrokerThePortNumber()) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the ServiceBroker");
			}
			return (false);		// async socket allows failures.
		}
		itsStatus = EP_WAIT_FOR_SB_ANSWER;
	}

	// third step: wait for the answer of the SB
	if (itsStatus == EP_WAIT_FOR_SB_ANSWER) {
		if (!_waitForSBAnswer()) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
		itsStatus = EP_KNOWS_DEST;
	}

	// fourth step: try to connect to the real socket.
	if (itsStatus == EP_KNOWS_DEST) {
		if (!_startConnectionToPeer()) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
		itsStatus = EP_CONNECTING;
	}

	// fifth step: wait for response of connection request
	if (itsStatus == EP_CONNECTING) {
		if (!_waitForPeerResponse()) {
			if (itsSyncComm) {
				ASSERTSTR(false, "Cannot contact the other party");
			}
			return (false);		// async socket allows failures.
		}
		itsStatus = EP_CONNECTED;
	}

	return (itsStatus == EP_CONNECTED);
}

//
// send a message to the message broker requesting a portnumber
//
bool EventPort::_askBrokerThePortNumber()
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

	return (true);
}

//
// waitForSBAnswer()
//
// Wait until we received a response from the service broker.
//
bool EventPort::_waitForSBAnswer()
{
	// wait for response
	GCFEvent*	answerEventPtr(receiveEvent(itsBrokerSocket));
	if (!answerEventPtr) {
		return (false);
	}

	// a complete event was received, handle it.
	if (itsIsServer) {
		SBServiceRegisteredEvent response(*answerEventPtr);
		if (response.result != 0) {
			LOG_ERROR_STR("Service " << itsServiceName << " is already on the air.");
			return (false);
		}
		itsPort = response.portnumber;
		LOG_DEBUG_STR("Service " << itsServiceName << " will be at port " << itsPort);
		// note: keep connection with Broker so he knows we are on the air.
	}
	else {	// client socket
		SBServiceInfoEvent response(*answerEventPtr);
		if (response.result != 0) {
			LOG_ERROR_STR("Service " << itsServiceName << " is unknown");
			return (false);
		}
		itsPort = response.portnumber;
		LOG_DEBUG_STR("Service " << itsServiceName << " is at port " << itsPort);

		// close connection with Broker.
		itsBrokerSocket->shutdown();
		itsBrokerSocket = 0;
	}

	return (true);
}

//
// _startConnectionToPeer()
//
bool EventPort::_startConnectionToPeer()
{
	// Finally we can make the real connection
	if (itsIsServer) {
		LOG_DEBUG_STR ("Opening listener on port " << itsPort);
		itsListenSocket = new Socket(itsServiceName, toString(itsPort), Socket::TCP);
		itsSocket = itsListenSocket->accept(itsSyncComm ? -1 : 500);
	}
	else {
		LOG_DEBUG_STR ("Trying to make connection with " << itsServiceName);
		itsSocket = new Socket(itsServiceName, itsHost, toString(itsPort), Socket::TCP);
		itsSocket->connect(itsSyncComm ? -1 : 500);	// try to connect, wait forever or 0.5 sec
	}
	return (true);
}

//
// _waitForPeerResponse()
//
bool EventPort::_waitForPeerResponse()
{
	// connection (already) successful?
	if (itsSocket->isConnected()) {
		itsSocket->setBlocking(itsSyncComm);
		return (true);
	}

	// do a retry
	if (itsIsServer) {
		itsSocket = itsListenSocket->accept(itsSyncComm ? -1 : 500);
	}
	else {
		itsSocket->connect(itsSyncComm ? -1 : 500);	// try to connect, wait forever or 0.5 sec
	}

	return (false);
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
// static sendEvent(Socket*, Event*)
//
void EventPort::sendEvent(Socket*		aSocket,
						  GCFEvent*		anEvent)
{
	// Serialize the message and write buffer to port
	uint32	packSize;
	void* 	buf 	   = anEvent->pack(packSize);
	int32 	btsWritten = aSocket->write(buf, packSize);
	ASSERTSTR(btsWritten == (int32)packSize, 
			  "Only " << btsWritten << " of " << packSize << " bytes written");
}

//
// static receiveEvent(aSocket)
//
GCFEvent*	EventPort::receiveEvent(Socket*	aSocket)
{
	static int32	gBtsToRead(0);
	static int32	gTotalBtsRead(0);
	static int32	gReadState(0);

	GCFEvent*		header ((GCFEvent*) &receiveBuffer[0]);
	int32			btsRead(0);

	// first read signal (= eventtype) field
	if (gReadState == 0) {
		btsRead = aSocket->read((void*) &(header->signal), sizeof(header->signal));
		if (btsRead != sizeof(header->signal)) {
			if (aSocket->isBlocking()) {
				ASSERTSTR(false, "Event-type was not received");
			}
			return (0);		// async socket allows failures.
		}
		gReadState++;
	}

	// next read the length of the rest of the message
	if (gReadState == 1) {
		btsRead = aSocket->read((void*) &(header->length), sizeof(header->length));
		if (btsRead != sizeof(header->length)) {
			if (aSocket->isBlocking()) {
				ASSERTSTR(false, "Event-length was not received");
			}
			return (0);		// async socket allows failures.
		}
		gReadState++;
		gBtsToRead = header->length;		// get size of data part
	}

	// finally read the datapart of the event
	if (gReadState == 2) {
		LOG_DEBUG_STR("Still " << gBtsToRead << " bytes of data to get");

		btsRead = 0;
		if (gBtsToRead) {
			btsRead = aSocket->read(&receiveBuffer[sizeof(GCFEvent) + gTotalBtsRead], gBtsToRead);
			if (btsRead != gBtsToRead) {
				if (aSocket->isBlocking()) {
					ASSERTSTR(false, "Only " << btsRead << " bytes of msg read: " << gBtsToRead);
				}
				return (0);		// async socket allow failures
			}
		}

		if (btsRead == gBtsToRead) {	// everything received?
			// reset own admin
			gReadState    = 0;
			gTotalBtsRead = 0;
			gBtsToRead    = 0;
//			hexdump(receiveBuffer, sizeof(GCFEvent) + header->length);
			return (header);
		}
	}

	// not all information received yet
	return (0L);
}

 
    } // namespace TM
  } // namespace GCF
} // namespace LOFAR
