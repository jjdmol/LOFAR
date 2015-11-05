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
#include <KVT_Protocol.ph>

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
	itsSeqnr		 (-1),
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


bool KVTLogger::log(const string& key, const string& value, double secsEpoch1970)
{
	if (!itsLoggingEnabled) {
		return (false);
	}

	KVTSendMsgEvent		logEvent;
	logEvent.seqnr     = --itsSeqnr;	// use negative seqnrs to avoid ack messages
	logEvent.key       = key;
	logEvent.value     = value;
	logEvent.timestamp = secsEpoch1970;
	itsKVTport->send(&logEvent);
	return (true);
}

bool KVTLogger::log(const vector<string> keys, const vector<string> values, const vector<double> times)
{
	if (!itsLoggingEnabled) {
		return (false);
	}

	size_t	nrElements = keys.size();
	if (values.size() != nrElements || times.size() != nrElements) {
		LOG_FATAL(formatString("Trying to send unequal length of vectors: k=%d, v=%d, t=%d",nrElements, values.size(), times.size()));
		return (false);
	}

	KVTSendMsgPoolEvent		logEvent;
	logEvent.seqnr     = --itsSeqnr;	// use negative seqnrs to avoid ack messages
	logEvent.nrElements= nrElements;
	logEvent.keys()    = keys;
	logEvent.values()  = values;
	logEvent.times()   = times;
	itsKVTport->send(&logEvent);
	return (true);
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

	LOG_DEBUG("Sending register event");
	KVTRegisterEvent	regEvent;
	regEvent.obsID = itsObsID;
	regEvent.name  = itsRegisterName;
	itsKVTport->send(&regEvent);

	LOG_DEBUG("Waiting for register acknowledgement");
	GCFEvent*	ackPtr;
	while ((ackPtr = itsKVTport->receive()) == 0) {		// may be a async socket.
		usleep(10);
	}
	KVTRegisterAckEvent	ack(*ackPtr);
	if (ack.obsID != itsObsID || ack.name != itsRegisterName) {
		LOG_FATAL_STR("IDENTITY ERROR: LOGGING OF KEY-VALUE PAIRS IS NOT POSSIBLE!");
		return (false);
	}

	LOG_INFO_STR("Connected to and registered at the KeyValueLogger");
	return (true);
}
 
  } // namespace MACIO
} // namespace LOFAR
