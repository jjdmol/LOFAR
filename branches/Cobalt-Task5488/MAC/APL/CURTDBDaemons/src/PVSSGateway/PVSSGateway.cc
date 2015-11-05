//#  PVSSGateway.cc: sets values in PVSS.
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
//#  $Id: PVSSGateway.cc 23417 2012-12-20 14:06:29Z loose $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/PosixTime.h>
#include <ApplCommon/StationInfo.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include "PVSSGateway.h"
#include "PVSSDatapointDefs.h"
#include <CURTDBDaemons/Package__Version.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <signal.h>

namespace LOFAR {
  using namespace MACIO;
  namespace GCF {
    using namespace TM;
    using namespace PVSS;
    using namespace RTDB;
    namespace RTDBDaemons {

using namespace boost::posix_time;

// static pointer to this object for signal handler
static PVSSGateway*	thisPVSSGateway = 0;

//
// PVSSGateway()
//
PVSSGateway::PVSSGateway(const string&	myName) :
	GCFTask((State)&PVSSGateway::initial, myName),
	itsListener  (0),
	itsDPservice (0),
	itsTimerPort (0)
{
	LOG_DEBUG_STR("PVSSGateway(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("PVSSGateway"));

	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);
	registerProtocol(KVT_PROTOCOL,	 KVT_PROTOCOL_STRINGS);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_PVSSGATEWAY, GCFPortInterface::MSPP, KVT_PROTOCOL);
	ASSERTSTR(itsListener, "Can't allocate a listener port");

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");
}

//
// ~PVSSGateway()
//
PVSSGateway::~PVSSGateway()
{
	LOG_DEBUG_STR("~PVSSGateway()");
	
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
// sigintHandler(signum)
//
void PVSSGateway::sigintHandler(int signum)
{
	LOG_INFO(formatString("SIGINT signal detected (%d)",signum));

	if (thisPVSSGateway) {
		thisPVSSGateway->finish();
	}
}

//
// finish()
//
void PVSSGateway::finish()
{
	TRAN(PVSSGateway::finish_state);
}

//
// initial(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult PVSSGateway::initial(GCFEvent& event, GCFPortInterface& port)
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
		TRAN(PVSSGateway::connect2PVSS);
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
// connect2PVSS(event, port)
//
// Try to connect to PVSS
//
GCFEvent::TResult PVSSGateway::connect2PVSS(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("connect2PVSS:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// Get access to my own propertyset.
		string propSetName(createPropertySetName(PSN_PVSS_GATEWAY, getName()));
		LOG_INFO_STR("Activating PropertySet" << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_PVSS_GATEWAY,
											 PSAT_WO,
											 this);
		// Wait for timer that is set on DP_CREATED event

		// Instruct loggingProcessor
		LOG_INFO_STR("MACProcessScope: " << propSetName);
	} break;

	case DP_CREATED: {
		// NOTE: thsi function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent  dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.5);    // give RTDB time to get original value.
	} break;

	case F_TIMER: {
		thisPVSSGateway = this;
		signal (SIGINT,  PVSSGateway::sigintHandler);	// ctrl-c
		signal (SIGTERM, PVSSGateway::sigintHandler);	// kill
		signal (SIGABRT, PVSSGateway::sigintHandler);	// kill -6

		// update PVSS
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Initial"));
		itsPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		
		TRAN(PVSSGateway::operational);
	} break;

	case DP_CHANGED:
		break;

	default:
		LOG_DEBUG_STR("connect2PVSS default: " << eventName(event) << "@" << port.getName());
		break;
	}

    return (GCFEvent::HANDLED);
}


//
// operational(event, port)
//
GCFEvent::TResult PVSSGateway::operational(GCFEvent&		event, GCFPortInterface&	port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Active"));
		itsTimerPort->setTimer(1.0, 5.0); 
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
		bool	sendOk(_addKVT(logEvent.kvp));
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
		LOG_DEBUG_STR("Received: " << logEvent);
		bool	sendOk(true);
		for (uint32 i = 0; i < logEvent.kvps.size(); i++) {
			sendOk &= _addKVT(logEvent.kvps[i]);
		}
		itsClients[&port].msgCnt += logEvent.kvps.size();
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
// finish_state(event,port)
//
GCFEvent::TResult PVSSGateway::finish_state(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("finish_state:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Finished"));
		itsPropertySet->setValue(PN_FSM_ERROR,			GCFPVString(""));

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
void PVSSGateway::_registerClient(GCFPortInterface&	port,
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
void PVSSGateway::_registerFailure(GCFPortInterface&		port)
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
// _KVPairType2PVSStype
//
PVSS::TMACValueType	PVSSGateway::_KVpairType2PVSStype(int	kvpType)
{
	switch (kvpType) {
	case KVpair::VT_STRING:	return (LPT_STRING);
	case KVpair::VT_BOOL:	return (LPT_BOOL);
	case KVpair::VT_INT:	return (LPT_INTEGER);
	case KVpair::VT_DOUBLE:	return (LPT_DOUBLE);
	case KVpair::VT_FLOAT:	return (LPT_DOUBLE);
	case KVpair::VT_TIME_T:	return (LPT_DATETIME);
	default:				return (LPT_STRING);		// best guess
	}
}

//
// _addKVT(key, value, time)
//
bool PVSSGateway::_addKVT(const KVpair&		kvp)
{
	PVSSresult result = itsDPservice->setValue(kvp.first, kvp.second, _KVpairType2PVSStype(kvp.valueType), kvp.timestamp, true);
	return (result == SA_NO_ERROR);
}


  } // namespace RTDBDaemons
 } // namespace GCF
} // namespace LOFAR
