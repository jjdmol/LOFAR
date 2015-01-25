//#  PVSSGatewayStub.cc: sets values in PVSS.
//#
//#  Copyright (C) 2013
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
//#  $Id: PVSSGatewayStub.cc 23417 2012-12-20 14:06:29Z loose $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <Common/NsTimestamp.h>
#include <ApplCommon/PosixTime.h>
//#include <ApplCommon/StationInfo.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include "PVSSGatewayStub.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <PVSSGateway_Stub/Package__Version.h>
#include <signal.h>

namespace LOFAR {
  using namespace MACIO;
  namespace GCF {
    using namespace TM;
    namespace CUDaemons {

using namespace boost::posix_time;

// static pointer to this object for signal handler
static PVSSGatewayStub*	thisPVSSGatewayStub = 0;

//
// PVSSGatewayStub()
//
PVSSGatewayStub::PVSSGatewayStub(const string&	myName) :
	GCFTask((State)&PVSSGatewayStub::initial, myName),
	itsListener 	(0),
	itsTimerPort 	(0),
	itsMsgBufTimer	(0)
{
	LOG_DEBUG_STR("PVSSGatewayStub(" << myName << ")");
	LOG_INFO(Version::getInfo<PVSSGateway_StubVersion>("PVSSGatewayStub"));

	registerProtocol(KVT_PROTOCOL,	 KVT_PROTOCOL_STRINGS);

	string		fileName = globalParameterSet()->getString("outputFile", "PVSSGatewayStub.output");

	// Try to open the file
	LOG_DEBUG_STR("Trying to create the outputfile `" << fileName << "'");
	itsOutputFile.open(fileName.c_str(), ofstream::out | ofstream::app);
	if (!itsOutputFile) {
		THROW (APSException, formatString("Unable to open file %s", fileName.c_str()));
	}

	// initialize the ports
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_PVSSGATEWAY, GCFPortInterface::MSPP, KVT_PROTOCOL);
	ASSERTSTR(itsListener, "Can't allocate a listener port");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");
}

//
// ~PVSSGatewayStub()
//
PVSSGatewayStub::~PVSSGatewayStub()
{
	LOG_DEBUG_STR("~PVSSGatewayStub()");
	
	if (itsTimerPort) {	
		delete itsTimerPort;
	}
	if (itsListener) {
		delete itsListener;
	}

	// Close the output file
	itsOutputFile.close();
}

//
// sigintHandler(signum)
//
void PVSSGatewayStub::sigintHandler(int signum)
{
	LOG_INFO(formatString("SIGINT signal detected (%d)",signum));

	if (thisPVSSGatewayStub) {
		thisPVSSGatewayStub->finish();
	}
}

//
// finish()
//
void PVSSGatewayStub::finish()
{
	TRAN(PVSSGatewayStub::finish_state);
}

//
// initial(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult PVSSGatewayStub::initial(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR("initial:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		if (!itsListener->isConnected()) {
			itsListener->open();
		}
	break;

	case F_CONNECTED:
		// Listener is opened, Connect to PVSS
		thisPVSSGatewayStub = this;
		signal (SIGINT,  PVSSGatewayStub::sigintHandler);	// ctrl-c
		signal (SIGTERM, PVSSGatewayStub::sigintHandler);	// kill
		signal (SIGABRT, PVSSGatewayStub::sigintHandler);	// kill -6
		TRAN(PVSSGatewayStub::operational);
	break;

	case F_DISCONNECTED:
		port.setTimer(5.0); // try again after 5 second
	break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// operational(event, port)
//
GCFEvent::TResult PVSSGatewayStub::operational(GCFEvent&		event, GCFPortInterface&	port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(1.0, 10.0); 
	break;

	// Catch incoming connections of new clients
	case F_ACCEPT_REQ: {
		GCFTCPPort*		client(new GCFTCPPort);
		if (!client) {
			LOG_ERROR("Can't allocate new socket for new client");
			return (status);
		}
		if (&port == itsListener) {
			client->init(*this, "application", GCFPortInterface::SPP, KVT_PROTOCOL);
			itsListener->accept(*client);
		}
	} break;

	case F_CONNECTED:
	break;

	case F_DISCONNECTED: {
		ASSERTSTR(itsListener != &port, "Lost listener-port, bailing out"); 
		LogClientMap::iterator	iter = itsClients.find(&port);
		if (iter == itsClients.end()) {
			LOG_INFO("Connection lost to a not-registered LofarLogger client.");
		}
		else {
			if (iter->second.valid) {
				LOG_INFO_STR("Closing log-stream with " << iter->second.name << ", passed " 
							<< iter->second.msgCnt << " messages to the database");
			}
			else if (!iter->second.name.empty()) {
				LOG_INFO_STR("Closing log-stream with " << iter->second.name);
			}
			else {
				LOG_INFO("Closing unknown log-stream");
			}
		}
		port.close();
		itsClients.erase(&port);
		itsClientsGarbage.push_back(&port);
	} break;

	case F_TIMER: {
		_garbageCollection();
	}  break;

	case KVT_REGISTER: {
		KVTRegisterEvent		registerEvent(event);
		_registerClient(port, registerEvent.name, registerEvent.obsID);

		KVTRegisterAckEvent		answer;
		answer.obsID = registerEvent.obsID;
		answer.name  = registerEvent.name;
		port.send(answer);
	} break;

	case KVT_SEND_MSG: {
		KVTSendMsgEvent		logEvent(event);
		LOG_DEBUG_STR("Received: " << logEvent);
		_writeKVT(logEvent.kvp);
		itsClients[&port].msgCnt++;

		if (logEvent.seqnr > 0) {
			KVTSendMsgAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = 0;
			port.send(answer);
		}
	} break;

	case KVT_SEND_MSG_POOL: {
		KVTSendMsgPoolEvent		logEvent(event);
		LOG_DEBUG_STR("Received: " << logEvent);
		for (size_t i = 0; i < logEvent.kvps.size(); i++) {
			_writeKVT(logEvent.kvps[i]);
		}
		itsClients[&port].msgCnt += logEvent.kvps.size();
		if (logEvent.seqnr > 0) {
			KVTSendMsgPoolAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = 0;
			port.send(answer);
		}
	} break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// finish_state(event,port)
//
GCFEvent::TResult PVSSGatewayStub::finish_state(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("finish_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(1.0);		// Give PVSS time to propagate the data change.
	} break;

	case F_TIMER:
		GCFScheduler::instance()->stop();

	default:
		LOG_INFO_STR("finish_state default: "  << eventName(event) << "@" << port.getName());
	}

	return (GCFEvent::HANDLED);
}

//
// _registerClient(port, name)
//
void PVSSGatewayStub::_registerClient(GCFPortInterface&	port,
									 const string&		name,
									 uint32				obsID)
{
	LogClientMap::iterator iter = itsClients.find(&port);
	if (iter != itsClients.end()) {
		LOG_WARN_STR("Client " << name << "is already registered");
		return;
	}

	itsClients[&port] = LogClient(name, obsID);
	itsClients[&port].obsID  = obsID;
	itsClients[&port].msgCnt = 0;
	itsClients[&port].valid  = true;
	LOG_INFO_STR("Starting KVT stream for " << name);

	return;
}

//
// _registerFailure(port)
//
void PVSSGatewayStub::_registerFailure(GCFPortInterface&		port)
{
	LogClientMap::iterator iter = itsClients.find(&port);
	if (iter == itsClients.end()) {
		return;
	}

	if (++(iter->second.errCnt) > 10) {
		iter->second.valid = false;
		LOG_INFO_STR("Log-stream to " << iter->second.name << " keeps reporting errors, ignoring stream");
	}
}

//
// _writeKVT(key, value, time)
//
void PVSSGatewayStub::_writeKVT(const KVpair&		kvp)
{
	itsOutputFile << kvp.first << "=" << kvp.second << "\n";
	itsOutputFile.flush();
	LOG_INFO(formatString("%s=%s\n",kvp.first.c_str(), kvp.second.c_str()));
}

// ---------------------------------------- client socket administration ----------------------------------------
//
// _garbageCollection()
//
void PVSSGatewayStub::_garbageCollection()
{
	// cleanup the garbage of closed ports to master clients
	if (itsClientsGarbage.empty()) {
		return;
	}

	LOG_DEBUG_STR("_garbageCollection:" << itsClientsGarbage.size());
	GCFPortInterface* pPort;
	for (TClients::iterator iter = itsClientsGarbage.begin();
		iter != itsClientsGarbage.end(); ++iter) {
		pPort = *iter;
		delete pPort;
	}
	itsClientsGarbage.clear();
}

  } // namespace RTDBDaemons
 } // namespace GCF
} // namespace LOFAR
