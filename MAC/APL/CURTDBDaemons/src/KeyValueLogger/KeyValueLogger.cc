//#  KeyValueLogger.cc: Filters and stores logmessages in PVSS
//#
//#  Copyright (C) 2007-2011
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

#include <lofar_config.h>
#include <ApplCommon/PosixTime.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <OTDB/ClassifConv.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/TreeValue.h>
#include "KeyValueLogger.h"
#include <CURTDBDaemons/Package__Version.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace LOFAR {
  using namespace MACIO;
  using namespace OTDB;
  namespace GCF {
    using namespace TM;
    using namespace PVSS;
    using namespace RTDB;
    namespace RTDBDaemons {

using namespace boost::posix_time;

//
// CodeloggingProcessor()
//
KeyValueLogger::KeyValueLogger(const string&	myName) :
	GCFTask((State)&KeyValueLogger::initial, myName),
	itsListener  (0),
	itsDPservice (0),
	itsSASservice(0),
	itsTimerPort (0)
{
	LOG_DEBUG_STR("KeyValueLogger(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("KeyValueLogger"));

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);
	registerProtocol(KVT_PROTOCOL,	 KVT_PROTOCOL_STRINGS);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_KVTLOGGER, GCFPortInterface::MSPP, KVT_PROTOCOL);
	ASSERTSTR(itsListener, "Can't allocate a listener port");

//	itsDPservice = new DPservice(this);
//	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

	itsSASdbname  = globalParameterSet()->getString("KeyValueLogger.OTDBdatabase");
	itsSAShostname= globalParameterSet()->getString("KeyValueLogger.OTDBhostname");
	itsSASservice = new OTDBconnection("paulus", "boskabouter", itsSASdbname, itsSAShostname);
	ASSERTSTR(itsSASservice, "Can't allocate connection to SAS");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");
}

//
// ~CodeloggingProcessor()
//
KeyValueLogger::~KeyValueLogger()
{
	LOG_DEBUG_STR("~KeyValueLogger()");
	
	if (itsTimerPort) {	
		delete itsTimerPort;
	}
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsSASservice) {
		delete itsSASservice;
	}
	if (itsListener) {
		delete itsListener;
	}
}

//
// initial(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult KeyValueLogger::initial(GCFEvent& event, GCFPortInterface& port)
{
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
		// Listener is opened, go to operational state.
		TRAN(KeyValueLogger::connect2SAS);
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
// connect2SAS(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult KeyValueLogger::connect2SAS(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("connect2SAS:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// Connect to the SAS database
		LOG_INFO(formatString("Trying to connect to SAS database %s@%s", itsSASdbname.c_str(), itsSAShostname.c_str()));
		itsSASservice->connect();
		if (!itsSASservice->isConnected()) {
			LOG_WARN("Connection to SAS failed. Retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			return (GCFEvent::HANDLED);
		}

		// Get ID of the current PIC tree
		LOG_INFO("Connection to SAS database succesfull, getting info about instrument");
		ClassifConv     CTconv(itsSASservice);
		TreeTypeConv    TTconv(itsSASservice);
		vector<OTDBtree>    treeList = itsSASservice->getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
		ASSERTSTR(treeList.size() == 1, "Expected 1 hardware tree in SAS, database error=" << itsSASservice->errorMsg());

		itsPICtreeID = treeList[0].treeID();
		LOG_INFO(formatString("Using PICtree %d (%s)", itsPICtreeID, to_simple_string(treeList[0].starttime).c_str()));

		// SAS part seems OK, continue with main loop
		itsKVTgate = new TreeValue(itsSASservice, itsPICtreeID);
		ASSERTSTR(itsKVTgate, "Unable to create a TreeValue object for tree: " << itsPICtreeID);

		TRAN(KeyValueLogger::operational);
	}
	break;

	default:
		break;
	}

    return (GCFEvent::HANDLED);
}


//
// operational(event, port)
//
GCFEvent::TResult KeyValueLogger::operational(GCFEvent&		event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static unsigned long garbageTimerID = 0;

	switch (event.signal) {
	case F_ENTRY:
		garbageTimerID = itsTimerPort->setTimer(1.0, 5.0); 
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
	}
	break;

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
	}
	break;

	case F_TIMER: {
		// cleanup the garbage of closed ports to master clients
		GCFPortInterface* pPort;
		for (TClients::iterator iter = itsClientsGarbage.begin();
			iter != itsClientsGarbage.end(); ++iter) {
			pPort = *iter;
			delete pPort;
		}
		itsClientsGarbage.clear();
	}  
	break;

	case KVT_REGISTER: {
		KVTRegisterEvent		registerEvent(event);
		_registerClient(port, registerEvent.name, registerEvent.obsID);

		KVTRegisterAckEvent		answer;
		answer.obsID = registerEvent.obsID;
		answer.name  = registerEvent.name;
		port.send(answer);
	}
	break;

	case KVT_SEND_MSG: {
		KVTSendMsgEvent		logEvent(event);
		LOG_DEBUG_STR("Received: " << logEvent);
		bool	sendOk(itsKVTgate->addKVT(logEvent.key, logEvent.value, from_ustime_t(logEvent.timestamp)));
		itsClients[&port].msgCnt++;

		if (logEvent.seqnr > 0) {
			KVTSendMsgAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = !sendOk;
			port.send(answer);
		}
	}
	break;


	case KVT_SEND_MSG_POOL: {
		KVTSendMsgPoolEvent		logEvent(event);
		if (logEvent.keys().size() != logEvent.nrElements || 
			logEvent.values().size() != logEvent.nrElements ||
			logEvent.times().size() != logEvent.nrElements) {
			LOG_ERROR(formatString("Received kvt pool from %s (seqnr=%d) with unequal vectorsizes (n=%d, k=%d, v=%d, t=%d)", 
					itsClients[&port].name.c_str(), logEvent.seqnr, logEvent.nrElements, 
					logEvent.keys().size(), logEvent.values().size(), logEvent.times().size()));
			if (logEvent.seqnr > 0) {
				KVTSendMsgPoolAckEvent	answer;
				answer.seqnr = logEvent.seqnr;
				answer.result = -1;
				port.send(answer);
				break;
			}
		}

		bool	sendOk(true);
		for (uint32 i = 0; i < logEvent.nrElements; i++) {
			sendOk &= itsKVTgate->addKVT(logEvent.keys()[i], logEvent.values()[i], from_ustime_t(logEvent.times()[i]));
		}
		itsClients[&port].msgCnt += logEvent.nrElements;
		if (logEvent.seqnr > 0) {
			KVTSendMsgPoolAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = !sendOk;	// bool -> int: 0=Ok
			port.send(answer);
		}
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// _registerClient(port, name)
//
void KeyValueLogger::_registerClient(GCFPortInterface&	port,
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
void KeyValueLogger::_registerFailure(GCFPortInterface&		port)
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


  } // namespace RTDBDaemons
 } // namespace GCF
} // namespace LOFAR
