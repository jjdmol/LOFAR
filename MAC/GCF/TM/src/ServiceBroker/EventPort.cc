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

static	char		receiveBuffer[24*4096];

//
// EventPort (name, type, protocol)
//
EventPort::EventPort(const string&		aServiceMask,
					 bool				aServerSocket,
					 int				aProtocol,
					 const string&		aHostname) :
	itsPort		   (0),
	itsHost		   (aHostname),
	itsSocket	   (0),
	itsListenSocket(0),
	itsBrokerSocket(0)
{
	if (itsHost.empty() || itsHost == "localhost") {
		itsHost = Common::myHostname(false);
	} 

	// First make a connection with the servicebroker and ask the SB on which
	// port the service is running.
	LOG_DEBUG ("Trying to make connection with the ServiceBroker");

	Socket*		itsBrokerSocket(new Socket("ServiceBroker", itsHost, toString(MAC_SERVICEBROKER_PORT)));
	ASSERTSTR(itsBrokerSocket, "can't allocate socket to serviceBroker");
	itsBrokerSocket->connect(-1);				// try to connect, wait forever
	itsBrokerSocket->setBlocking(true);		// no other tasks, do rest blocking

	string	serviceName(makeServiceName(aServiceMask, 0));
	// construct the question.
	if (aServerSocket) {
		SBRegisterServiceEvent	request;
		request.seqnr		= 5;
		request.servicename	= serviceName;

		// send question
		sendEvent(itsBrokerSocket, &request);

		// wait for response
		SBServiceRegisteredEvent response(receiveEvent(itsBrokerSocket));
		ASSERTSTR(response.result == 0, "Service " << serviceName << " can not be used");
		itsPort = response.portnumber;
		LOG_DEBUG_STR("Service " << serviceName << " will be at port " << itsPort);

		// note: keep connection with Broker so he knows we are in the air.
	}
	else {
		SBGetServiceinfoEvent	request;
		request.seqnr		= 5;
		request.servicename	= serviceName;
		request.hostname	= aHostname;

		// send question
		sendEvent(itsBrokerSocket, &request);

		// wait for response
		SBServiceInfoEvent response(receiveEvent(itsBrokerSocket));
		ASSERTSTR(response.result == 0, "Service " << serviceName << " is unknown");
		itsPort = response.portnumber;
		LOG_DEBUG_STR("Service " << serviceName << " is at port " << itsPort);

		// close connection with Broker.
		itsBrokerSocket->shutdown();
		itsBrokerSocket = 0;
	}

	// Finally we can make the real connection
	if (aServerSocket) {
		LOG_DEBUG_STR ("Opening listener on port " << itsPort);
		itsListenSocket = new Socket(serviceName, toString(itsPort), Socket::TCP);
		itsSocket = itsListenSocket->accept(-1);
		itsSocket->setBlocking(false);		// assume async.
	}
	else {
		LOG_DEBUG_STR ("Trying to make connection with " << serviceName);
		itsSocket = new Socket(serviceName, itsHost, toString(itsPort), Socket::TCP);
		itsSocket->connect(1000);
		itsSocket->setBlocking(false);		// assume async.
	}

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
// send(Event*)
//
bool EventPort::send(GCFEvent*	anEvent)
{
	if (!itsSocket->isConnected() && !itsSocket->connect()) {
		return (false);
	}

	sendEvent(itsSocket, anEvent);
	return (true);
}

//
// receive() : Event
//
GCFEvent&	EventPort::receive()
{
	return (receiveEvent(itsSocket));
}

//
// makeServiceName(mask, number)
//
string	EventPort::makeServiceName(const string&	aServiceMask, int32		aNumber)
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
GCFEvent&	EventPort::receiveEvent(Socket*	aSocket)
{
	// First read header if answer:
	// That is signal field + length field.
	GCFEvent*	header = (GCFEvent*) &receiveBuffer[0];
	int32		btsRead;
	btsRead = aSocket->read((void*) &(header->signal), sizeof(header->signal));
	ASSERTSTR(btsRead == sizeof(header->signal), "Only " << btsRead << " of " 
						<< sizeof(header->signal) << " bytes of header read");
	btsRead = aSocket->read((void*) &(header->length), sizeof(header->length));
	ASSERTSTR(btsRead == sizeof(header->length), "Only " << btsRead << " of " 
						<< sizeof(header->length) << " bytes of header read");

	LOG_DEBUG("Header received");

	// Is there a payload in the message? This should be the case!
	int32	remainingBytes = header->length;
	LOG_DEBUG_STR(remainingBytes << " bytes to get next");
	if (remainingBytes <= 0) {
		return(*header);
	}

	// read remainder
	btsRead = aSocket->read(&receiveBuffer[sizeof(GCFEvent)], remainingBytes);
	ASSERTSTR(btsRead == remainingBytes,
		  "Only " << btsRead << " bytes of msg read: " << remainingBytes);

//	hexdump(receiveBuffer, sizeof(GCFEvent) + remainingBytes);

	// return Eventinformation
	return (*header);
}

 
    } // namespace TM
  } // namespace GCF
} // namespace LOFAR
