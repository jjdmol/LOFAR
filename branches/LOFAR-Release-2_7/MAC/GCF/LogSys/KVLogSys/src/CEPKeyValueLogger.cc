//#  CEPKeyValueLogger.cc: 
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

#include <GCF/GCF_ServiceInfo.h>
#include <GCF/TM/EventPort.h>
#include <GCF/LogSys/CEPKeyValueLogger.h>
#include <KVL_Protocol.ph>
#include <KVLDefines.h>
#include <sys/time.h>
#include <time.h>

namespace LOFAR {
  namespace GCF {
	using namespace TM;
	using namespace Common;
	namespace LogSys {

//
// CEPKeyValueLogger()
//
CEPKeyValueLogger::CEPKeyValueLogger() :
	_manIdToSkip(-1)
{
	// initialize the port
	itsKVLConn = new EventPort(MAC_SVCMASK_KVLDAEMON, false, KVL_PROTOCOL);
}


//
// logKeyValue(key, value, origin, timestamp, description)
//
void CEPKeyValueLogger::logKeyValue(const string& key, const GCFPValue& value, 
                                    TKVLOrigin origin, const timeval& timestamp, 
                                    const string& description)
{
	KVLUpdateEvent* pIndication = new KVLUpdateEvent;
	pIndication->key 			= key;
	pIndication->value._pValue	= &value;
	pIndication->origin 		= origin;
	pIndication->timestamp 		= timestamp;
	pIndication->description 	= description;

	if (itsKVLConn->send(pIndication)) {
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
void CEPKeyValueLogger::logKeyValue(const string& key, const GCFPValue& value, 
                                    TKVLOrigin origin, const string& description)
{
	timeval 			timestamp;
	struct timezone 	timeZone;
	gettimeofday(&timestamp, &timeZone); 

	logKeyValue(key, value, origin, timestamp, description);
}

//
// addAction(key, action, origin, timestamp, description)
//
void CEPKeyValueLogger::addAction(const string& key, uint8 action, 
                                  TKVLOrigin origin, timeval timestamp, 
                                  const string& description)
{
	if (timestamp.tv_sec == 0 && timestamp.tv_usec == 0) {
		struct timezone timeZone;
		gettimeofday(&timestamp, &timeZone); 
	}

	KVLAddActionEvent* pIndication = new KVLAddActionEvent;
	pIndication->key = key;
	pIndication->action = action;
	pIndication->origin = origin;
	pIndication->timestamp = timestamp;
	pIndication->description = description;

	if (itsKVLConn->send(pIndication)) {
		delete pIndication;
	}
	else {
		_msgQueue.push_back(pIndication);    
	}
}

//
// skipUpdatesFrom(manId)
//
void CEPKeyValueLogger::skipUpdatesFrom(uint8 manId)
{
	KVLSkipUpdatesFromEvent indication;
	indication.man_id = manId;
	_manIdToSkip = manId;
	itsKVLConn->send(&indication);		// ignore return value!
}

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR
