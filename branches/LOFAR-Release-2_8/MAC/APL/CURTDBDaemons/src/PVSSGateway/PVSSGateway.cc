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
#include <Common/NsTimestamp.h>
#include <ApplCommon/PosixTime.h>
#include <ApplCommon/StationInfo.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/PVSS/PVSSinfo.h>
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
	itsListener 	(0),
	itsDPservice 	(0),
	itsTimerPort 	(0),
	itsMsgBufTimer	(0),
	itsRemovalDelay (30.0),
	itsFlushInterval(5.0),
	itsMaxExpandSize(10)
{
	LOG_DEBUG_STR("PVSSGateway(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("PVSSGateway"));

	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);
	registerProtocol(KVT_PROTOCOL,	 KVT_PROTOCOL_STRINGS);

	itsRemovalDelay  = globalParameterSet()->getDouble ("removalDelay", 30.0);
	itsFlushInterval = globalParameterSet()->getDouble ("flushInterval", 5.0);
	itsMaxExpandSize = globalParameterSet()->getInt32  ("expandSize", 100);

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
		itsTimerPort->setTimer(1.0, itsFlushInterval); 
		itsMsgBufTimer = itsTimerPort->setTimer(0.5, 0.2);
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
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsMsgBufTimer) {	// fast, multiple times per second
			_processMsgBuffer();
		}
		else {	// slow timer, once every few seconds
			_garbageCollection();
			_flushValueCache();
			_cleanValueCache();
		}
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
		bool	sendOk(_add2MsgBuffer(logEvent.kvp));
		itsClients[&port].msgCnt++;

		if (logEvent.seqnr > 0) {
			KVTSendMsgAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = !sendOk;
			port.send(answer);
		}
	} break;

	case KVT_SEND_MSG_POOL: {
		KVTSendMsgPoolEvent		logEvent(event);
		LOG_DEBUG_STR("Received: " << logEvent);
		bool	sendOk(true);
		for (size_t i = 0; i < logEvent.kvps.size(); i++) {
			sendOk &= _add2MsgBuffer(logEvent.kvps[i]);
		}
		itsClients[&port].msgCnt += logEvent.kvps.size();
		if (logEvent.seqnr > 0) {
			KVTSendMsgPoolAckEvent	answer;
			answer.seqnr  = logEvent.seqnr;
			answer.result = !sendOk;	// bool -> int: 0=Ok
			port.send(answer);
		}
	} break;

	case DP_GET: {
		DPGetEvent		dpgEvent(event);
		string	DPname(PVSSinfo::getDPbasename(dpgEvent.DPname));	// strip off DBname
		LOG_DEBUG_STR("Add to ValueCache: " << DPname);
		dynArr_t		DA;
		DA.lastModify = NsTimestamp::now();
		DA.lastFlush  = DA.lastModify;
		DA.valArr 	  = (GCFPVDynArr*)(dpgEvent.value._pValue->clone());
		DA.valType	  = (TMACValueType) (dpgEvent.value._pValue->getType() & ~LPT_DYNARR);
		itsValueCache[DPname] = DA;
		_adoptRequestPool(DPname);
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
// _writeKVT(key, value, time)
//
bool PVSSGateway::_writeKVT(const KVpair&		kvp)
{
	PVSSresult result = itsDPservice->setValue(kvp.first, kvp.second, _KVpairType2PVSStype(kvp.valueType), kvp.timestamp, true);
	return (result == SA_NO_ERROR);
}

// ---------------------------------------- MSGbuffer administration ----------------------------------------
//
// _add2MsgBuffer(KVpair)
//
bool PVSSGateway::_add2MsgBuffer(const KVpair& kvp)
{
	if ((kvp.first.find('[')!=string::npos) && (kvp.first.find(']')!=string::npos)) {
		itsMsgBuffer.push(kvp);
		return (true);
	}

	return (_writeKVT(kvp));
}


//
// _processMsgBuffer()
//
// process all KV events that are in the MsgBuffer at the moment.
//
void PVSSGateway::_processMsgBuffer()
{
	while (!itsMsgBuffer.empty()) {
		KVpair	kvp = itsMsgBuffer.front();
		itsMsgBuffer.pop();
		// plain variable?
		string::size_type pos = kvp.first.find('[');
		if (pos == string::npos) {
			_writeKVT(kvp);			// just write and forget.
			break;
		}
		// its an dynarray.
		string	keyName(kvp.first.substr(0,pos));
		VCiter	cacheIter = itsValueCache.find(keyName);
		// seen it before?
		if (cacheIter == itsValueCache.end()) {
			// no, get the value from PVSS if not already requested....
			if (itsRequestBuffer.find(keyName) == itsRequestBuffer.end()) {
				itsDPservice->getValue(keyName);
			}
			// park valuechange.
			itsRequestBuffer.insert(make_pair(keyName, kvp));
		}
		else {	// update the element in the valueBuffer
			string::size_type epos = kvp.first.find(']', pos);
			if (epos == string::npos) {
				LOG_ERROR_STR("Ill formatted key will not be written: " << kvp.first);
				break;
			}
			int	index = atoi(kvp.first.substr(pos+1,epos).c_str());
			_setIndexedValue(keyName, index, kvp.second);
		}
	}
}

// ---------------------------------------- requestPool administration ----------------------------------------
//
// _adoptRequestPool(dpgEvent.DPname);
//
void PVSSGateway::_adoptRequestPool(const string& DPname)
{
	LOG_DEBUG_STR("_adoptRequestPool(" << DPname << ")");
	multimap<string,KVpair>::iterator	iter = itsRequestBuffer.begin();
	multimap<string,KVpair>::iterator	end  = itsRequestBuffer.end();
	while (iter != end) {
		if (iter->first == DPname) {
			string::size_type pos = iter->second.first.find('[');
			string::size_type epos = iter->second.first.find(']', pos);
			int	index = atoi(iter->second.first.substr(pos+1,epos).c_str());
			_setIndexedValue(DPname, index, iter->second.second);

			multimap<string,KVpair>::iterator	obsolete = iter;
			++iter;
			itsRequestBuffer.erase(obsolete);
		}
		else {
			++iter;
		}
	}
}

// ---------------------------------------- valueCache administration ----------------------------------------
//
// _setIndexedValue(keyname, index, value)
//
bool PVSSGateway::_setIndexedValue(const string& keyName, uint	index, const string&	value)
{
	// search the requested dynArray
	VCiter	cacheIter = itsValueCache.find(keyName);
	if (cacheIter == itsValueCache.end()) {
		LOG_ERROR_STR(keyName << " not in valueCache! Cannot set element " << index << " to " << value);
		return (false);
	}

	// check its size
	// Note: Normally you would not allow updating elements that are beyond the size of an array but the problem is that
	//       the database might contain new DP's that are still empty, so these could never be updated. Therefor we MUST
	//       allow writing values outside the current size of the dynArray. Since updates come in randomly we cannot grow
	//       the dynArray element by element (e.g. index 23 may occure before index 4). 
	// Solution: To protect the dynArray against unlimited grow (when a faulthy index was received) the maxsize of a dynArray
	//           is limited to itsMaxExpandSize (userdefined). When the current dynArray size is smaller than the requested
	//           index the dynarray is extended with elements till is can handle the index (limited to itsMaxExpandSize)
	//	   Note: The implementation still allows updates of dynArrays that have more than itsMaxExpandSize elements when the
	//           database already contained more than itsMaxExpandSize elements, it just limits the creation of new elements.
	if (index >= cacheIter->second.valArr->count()) {
		if (index > itsMaxExpandSize) {
			LOG_ERROR_STR(keyName << " has " << cacheIter->second.valArr->count() << " elements, grow to " << index << " is not allowed");
			return (false);
		}
		// add 'empty' elements till the index-th element fits.
		for (int i = index - cacheIter->second.valArr->count(); i >= 0; --i) {
			cacheIter->second.valArr->push_back(GCFPValue::createMACTypeObject(cacheIter->second.valType));
		}
	}

	// finally update the value
	(cacheIter->second.valArr->getValue()[index])->setValue(value);
	cacheIter->second.lastModify = NsTimestamp::now();
	LOG_DEBUG_STR("ValueCache: " << cacheIter->first << "[" << index << "]=" << value);
	return(true);
}

//
// _flushValueCache()
//
void PVSSGateway::_flushValueCache()
{
	double	now(NsTimestamp::now());
	VCiter	iter = itsValueCache.begin();
	VCiter	end  = itsValueCache.end();
	while (iter != end) {
		if (iter->second.lastModify > iter->second.lastFlush) {
			itsDPservice->setValue(iter->first, *(iter->second.valArr)); 
			iter->second.lastFlush = now;
			LOG_DEBUG_STR("ValueCache: flushed " << iter->first);
		}
		++iter;
	}

}

//
// _cleanValueCache()
//
void PVSSGateway::_cleanValueCache()
{
	double 	obsoleteTime = NsTimestamp::now() - itsRemovalDelay;
	VCiter	iter = itsValueCache.begin();
	VCiter	end  = itsValueCache.end();
	while (iter != end) {
		if (iter->second.lastModify < obsoleteTime) {
			VCiter	expired = iter;
			++iter;
			LOG_DEBUG_STR("ValueCache: remove " << expired->first);
			itsValueCache.erase(expired);
		}
		else {
			++iter;
		}
	}
}

// ---------------------------------------- client socket administration ----------------------------------------
//
// _garbageCollection()
//
void PVSSGateway::_garbageCollection()
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
