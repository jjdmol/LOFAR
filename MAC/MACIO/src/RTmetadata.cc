//#  RTmetadata.cc: (raw) socket based implementation to exchange Events
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
//#  $Id: RTmetadata.cc 14961 2010-02-10 15:51:20Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/RTmetadata.h>
#include <KVT_Protocol.ph>

namespace LOFAR {
  namespace MACIO {

//
// RTmetadata (name, type, protocol)
//
RTmetadata::RTmetadata(uint32			observationID,
					   const string&	registrationName,
					   const string&	hostName) :
	itsObsID		 (observationID),
	itsRegisterName	 (registrationName),
	itsLoggingEnabled(false),
	itsSeqnr		 (-1),
	itsKVTport		 (0)
{
	// Try to setup a connection with the PVSSGateway
	itsKVTport = new EventPort(MAC_SVCMASK_PVSSGATEWAY, false, KVT_PROTOCOL, hostName, true);
	ASSERTSTR(itsKVTport, "can't allocate socket to serviceBroker");

	itsLoggingEnabled = _setupConnection();
}

//
// ~RTmetadata
//
RTmetadata::~RTmetadata()
{
	if (itsKVTport) {
		delete itsKVTport;
	};
}


//
// log(KVpair)
//
bool RTmetadata::log(const KVpair&	kvp)
{
	if (!itsLoggingEnabled) {
		return (false);
	}

	KVTSendMsgEvent		logEvent;
	logEvent.seqnr = --itsSeqnr;	// use negative seqnrs to avoid ack messages
	logEvent.kvp   = kvp;
	itsKVTport->send(&logEvent);
	return (true);
}

//
// log(vector<KVpair>)
//
bool RTmetadata::log(const vector<KVpair> pairs)
{
	if (!itsLoggingEnabled) {
		return (false);
	}

	KVTSendMsgPoolEvent		logEvent;
	logEvent.seqnr = --itsSeqnr;	// use negative seqnrs to avoid ack messages
	logEvent.kvps  = pairs;
	itsKVTport->send(&logEvent);
	return (true);
}

//
// log(vector<key>, vector<value>, vector<timestamp>)
//
bool RTmetadata::log(const vector<string> keys, const vector<string> values, const vector<double> times)
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
	for (size_t i = 0; i < nrElements; i++) {
		logEvent.kvps.push_back(KVpair(keys[i],values[i],times[i]));
	}
	itsKVTport->send(&logEvent);
	return (true);
}

// -------------------- Internal routines --------------------

//
// _setupConnection()
//
bool RTmetadata::_setupConnection()
{
	if (!itsKVTport->connect()) {
		LOG_FATAL_STR("CONNECT ERROR: LOGGING OF KEY-VALUE PAIRS IS NOT POSSIBLE!");
		return (false);
	}

	LOG_INFO("Registering at PVSSGateway");
	KVTRegisterEvent	regEvent;
	regEvent.obsID = itsObsID;
	regEvent.name  = itsRegisterName;
	itsKVTport->send(&regEvent);

	LOG_INFO("Waiting for register acknowledgement");
	GCFEvent*	ackPtr;
	while ((ackPtr = itsKVTport->receive()) == 0) {		// may be a async socket.
		usleep(10);
	}
	KVTRegisterAckEvent	ack(*ackPtr);
	if (ack.obsID != itsObsID || ack.name != itsRegisterName) {
		LOG_FATAL_STR("IDENTITY ERROR: LOGGING OF KEY-VALUE PAIRS IS NOT POSSIBLE!");
		return (false);
	}

	LOG_INFO_STR("Connected to and registered at the PVSSGateway");
	return (true);
}
 
  } // namespace MACIO
} // namespace LOFAR
