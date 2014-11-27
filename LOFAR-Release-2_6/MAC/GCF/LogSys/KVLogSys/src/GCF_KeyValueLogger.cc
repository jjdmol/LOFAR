//#  GCF_KeyValueLogger.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include <GCF/GCF_ServiceInfo.h>
#include <GCF/LogSys/GCF_KeyValueLogger.h>
#include <Common/ParameterSet.h>
#include <KVL_Protocol.ph>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>

namespace LOFAR {

  namespace GCF {
	using namespace TM;
	using namespace Common;
	namespace LogSys {

GCFKeyValueLogger* GCFKeyValueLogger::_pInstance = 0;

//
// instance()
//
GCFKeyValueLogger* GCFKeyValueLogger::instance()
{
	if (!_pInstance) {
		_pInstance = new GCFKeyValueLogger();
		_pInstance->start();
	}

	return (_pInstance);
}

//
// GCFKeyValueLogger()
//
GCFKeyValueLogger::GCFKeyValueLogger() :
	GCFTask((State)&GCFKeyValueLogger::initial, KVL_CLIENT_TASK_NAME),
	_manIdToSkip(-1)
{
	// register the protocol for debugging purposes
	registerProtocol(KVL_PROTOCOL, KVL_PROTOCOL_signalnames);

	// initialize the port
	_kvlClientPort.init(*this, MAC_SVCMASK_KVLDAEMON, GCFPortInterface::SAP, KVL_PROTOCOL);
//	globalParameterSet()->adoptFile("../etc/KeyValueLoggerDaemon.conf");
}


//
// initial(event, port)
//
GCFEvent::TResult GCFKeyValueLogger::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (e.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		_kvlClientPort.open();
		break;

	case F_CONNECTED:
		TRAN(GCFKeyValueLogger::operational);
		break;

	case F_DISCONNECTED:
		_kvlClientPort.setTimer(TO_TRY_RECONNECT); // try again after 1 second
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// operational(event, port)
//
GCFEvent::TResult GCFKeyValueLogger::operational(GCFEvent& e, GCFPortInterface& p)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_DISCONNECTED:
		LOG_FATAL("Connection lost to KeyValue Logger deamon");
		p.close();
		break;

	case F_CLOSED:
		TRAN(GCFKeyValueLogger::initial);
		break;

	case F_ENTRY: {    
		KVLUpdateEvent* pUpdateEvent;
		GCFEvent* pEvent;
		for (TMsgQueue::iterator iter = _msgQueue.begin();
								iter != _msgQueue.end(); ++iter) {
			pEvent = *iter;
			_kvlClientPort.send(*pEvent);
			if (pEvent->signal == KVL_UPDATE) {
				pUpdateEvent = (KVLUpdateEvent*)pEvent;
				delete pUpdateEvent->value._pValue;
			}
			delete pEvent;
		}
		_msgQueue.clear();
		if (_manIdToSkip > -1) {
			skipUpdatesFrom(_manIdToSkip);
		}
		break;
	}  
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// logKeyValue(key, value, origin, timestamp, description)
//
void GCFKeyValueLogger::logKeyValue(const string& key, const GCFPValue& value, 
                                    TKVLOrigin origin, const timeval& timestamp, 
                                    const string& description)
{
	KVLUpdateEvent* pIndication = new KVLUpdateEvent;
	pIndication->key 			= key;
	pIndication->value._pValue	= &value;
	pIndication->origin 		= origin;
	pIndication->timestamp 		= timestamp;
	pIndication->description 	= description;

	if (_kvlClientPort.isConnected()) {
		_kvlClientPort.send(*pIndication);
		delete pIndication;
	}
	else {
		pIndication->value._pValue = value.clone();
		_msgQueue.push_back(pIndication);    
	}
}

//
// logKeyValue(key, value, origin, description)
//
void GCFKeyValueLogger::logKeyValue(const string& key, const GCFPValue& value, 
                                    TKVLOrigin origin, const string& description)
{
	timeval 			timestamp;
	struct timezone 	timeZone;
	gettimeofday(&timestamp, &timeZone); // TODO: find out whether this is the utc time or not

	logKeyValue(key, value, origin, timestamp, description);
}

//
// addAction(key, action, origin, timestamp, description)
//
void GCFKeyValueLogger::addAction(const string& key, uint8 action, 
                                  TKVLOrigin origin, timeval timestamp, 
                                  const string& description)
{
	if (timestamp.tv_sec == 0 && timestamp.tv_usec == 0) {
		struct timezone timeZone;
		gettimeofday(&timestamp, &timeZone); // TODO: find out whether this is the utc time or not
	}

	KVLAddActionEvent* pIndication = new KVLAddActionEvent;
	pIndication->key = key;
	pIndication->action = action;
	pIndication->origin = origin;
	pIndication->timestamp = timestamp;
	pIndication->description = description;

	if (_kvlClientPort.isConnected()) {
		_kvlClientPort.send(*pIndication);
		delete pIndication;
	}
	else {
		_msgQueue.push_back(pIndication);    
	}
}

//
// skipUpdatesFrom(manId)
//
void GCFKeyValueLogger::skipUpdatesFrom(uint8 manId)
{
	KVLSkipUpdatesFromEvent indication;
	indication.man_id = manId;
	_manIdToSkip = manId;
	if (_kvlClientPort.isConnected()) {
		_kvlClientPort.send(indication);
	}
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
