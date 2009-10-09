//#  KeyValueLogger.cc: Filters and stores logmessages in PVSS
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include "KeyValueLogger.h"
#include "../Package__Version.h"

namespace LOFAR {
  using namespace MACIO;
  namespace GCF {
    using namespace TM;
    using namespace PVSS;
    using namespace RTDB;
    namespace RTDBDaemons {

//
// CodeloggingProcessor()
//
KeyValueLogger::KeyValueLogger(const string&	myName) :
	GCFTask((State)&KeyValueLogger::initial, myName),
	itsListener (0),
	itsDPservice(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("KeyValueLogger(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("KeyValueLogger"));

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);
	registerProtocol(KVT_PROTOCOL,	 KVT_PROTOCOL_STRINGS);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_KVTLOGGER, GCFPortInterface::MSPP, KVT_PROTOCOL);
	ASSERTSTR(itsListener, "Can't allocate a listener port");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

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
	GCFEvent::TResult status = GCFEvent::HANDLED;
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
		TRAN(KeyValueLogger::operational);
	break;

	case F_DISCONNECTED:
		port.setTimer(5.0); // try again after 5 second
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return (status);
}

//
// operational(event, port)
//
GCFEvent::TResult KeyValueLogger::operational(GCFEvent&			event, 
												GCFPortInterface&	port)
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
				LOG_INFO_STR("Closing log-stream to " << iter->second.name << ", passed " 
							<< iter->second.msgCnt << " messages to the database");
			}
			else if (!iter->second.name.empty()) {
				LOG_INFO_STR("Closing log-stream to " << iter->second.name);
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
		KVTSendMsgAckEvent	answer;
		answer.seqnr  = logEvent.seqnr;
		answer.result = PVSS::SA_NO_ERROR;

		int32	timeStamp = indexValue(logEvent.key, "{}");
		rtrim  (logEvent.key, "{}01234565789");	 // cut off timestamp
		// replace all but last . with underscore.
		string::reverse_iterator	riter	= logEvent.key.rbegin();
		string::reverse_iterator	rend	= logEvent.key.rend();
		bool	lastDot(true);
		while (riter != rend) {
			if (*riter == '.') {
				if (lastDot) {
					lastDot = false;
				}
				else {
					*riter = '_';
				}
			}
			riter++;
		}
		PVSSresult	result = itsDPservice->setValue(logEvent.key, 
													GCFPVString(logEvent.value),
													1.0 * timeStamp);
		itsClients[&port].msgCnt++;

		switch (result) {
		case PVSS::SA_NO_ERROR:
			break;
		case PVSS::SA_SCADA_NOT_AVAILABLE:
			answer.result = result;
			break;
		default:
			// _registerFailure(port);
			answer.result = result;
		}
		port.send(answer);
	}
	break;

	case KVT_SEND_MSG_POOL: {
		KVTSendMsgPoolEvent		logEvent(event);
		KVTSendMsgPoolAckEvent	answer;
		answer.seqnr  = logEvent.seqnr;
		answer.result = PVSS::SA_NO_ERROR;
		for (uint32 i = 0; i < logEvent.msgCount; i++) {
			int32	timeStamp = indexValue(logEvent.keys.theVector[i], "{}");
			rtrim  (logEvent.keys.theVector[i], "{}01234565789");	 // cut off timestamp
			// replace all but last . with underscore.
			string::reverse_iterator	riter	= logEvent.keys.theVector[i].rbegin();
			string::reverse_iterator	rend	= logEvent.keys.theVector[i].rend();
			bool	lastDot(true);
			while (riter != rend) {
				if (*riter == '.') {
					if (lastDot) {
						lastDot = false;
					}
					else {
						*riter = '_';
					}
				}
				riter++;
			}
			PVSSresult	result = itsDPservice->setValue(logEvent.keys.theVector[i], 
														GCFPVString(logEvent.values.theVector[i]),
														1.0 * timeStamp);
			itsClients[&port].msgCnt++;

			switch (result) {
			case PVSS::SA_NO_ERROR:
				break;
			case PVSS::SA_SCADA_NOT_AVAILABLE:
				answer.result = result;
				i = logEvent.msgCount;		// don't try the others.
				break;
			default:
				// _registerFailure(port);
				answer.result |= result;
			} // switch
		} // for all msgs in the pool
		port.send(answer);
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
