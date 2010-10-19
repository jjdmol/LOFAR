//#  KVTLogger.cc: (raw) socket based implementation to exchange Events
//#
//#  Copyright (C) 2010
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
//#  $Id: KVTLogger.cc 14961 2010-02-10 15:51:20Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVTLogger.h>
#include <MACIO/KVT_Protocol.ph>

namespace LOFAR {
  namespace MACIO {

//
// KVTLogger (name, type, protocol)
//
KVTLogger::KVTLogger(uint32				observationID,
					 const string&		registrationName,
					 const string&		hostName,
					 bool				syncCommunication) :
	itsObsID		 (observationID),
	itsRegisterName	 (registrationName),
	itsLoggingEnabled(false),
	itsSeqnr		 (0),
	itsKVTport		 (0)
{
	// Try to setup a connection with the KeyValueLogger
	itsKVTport = new EventPort(MAC_SVCMASK_KVTLOGGER, false, KVT_PROTOCOL, hostName,  syncCommunication);
	ASSERTSTR(itsKVTport, "can't allocate socket to serviceBroker");

	itsLoggingEnabled = _setupConnection();
}

//
// ~KVTLogger
//
KVTLogger::~KVTLogger()
{
	if (itsKVTport) {
		delete itsKVTport;
	};
}


void KVTLogger::log(const string& key, const string& value, double secsEpoch1970)
{
	if (!itsLoggingEnabled) {
		return;
	}

	KVTSendMsgEvent		logEvent;
	logEvent.seqnr = ++itsSeqnr;
	logEvent.key   = formatString("%s{%20.6f}", key.c_str(), secsEpoch1970);
	logEvent.value = value;
	itsKVTport->send(&logEvent);
}

// -------------------- Internal routines --------------------

//
// _setupConnection()
//
bool KVTLogger::_setupConnection()
{
	if (!itsKVTport->connect()) {
		LOG_FATAL_STR("CONNECT ERROR: LOGGING OF KEY-VALUE PAIRS IS NOT POSSIBLE!");
		return (false);
	}

	KVTRegisterEvent	regEvent;
	regEvent.obsID = itsObsID;
	regEvent.name  = itsRegisterName;

	itsKVTport->send(&regEvent);

	GCFEvent*	ackPtr;
	ackPtr = itsKVTport->receive();
	KVTRegisterAckEvent	ack(*ackPtr);
	if (ack.obsID != itsObsID || ack.name != itsRegisterName) {
		LOG_FATAL_STR("LOGGING OF KEY-VALUE PAIRS IS NOT POSSIBLE!");
		return (false);
	}

	return (true);
}
 
  } // namespace MACIO
} // namespace LOFAR
