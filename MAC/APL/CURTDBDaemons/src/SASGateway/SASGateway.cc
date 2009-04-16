//#  SASGateway.cc: Filters and stores logmessages in PVSS
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
#include <ApplCommon/StationInfo.h>
#include <MACIO/GCF_Event.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <OTDB/ClassifConv.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/TreeValue.h>
#include "SASGateway.h"
#include "../Package__Version.h"

namespace LOFAR {
  using namespace MACIO;
  using namespace OTDB;
  namespace GCF {
    using namespace TM;
    using namespace PVSS;
    using namespace RTDB;
    namespace RTDBDaemons {

//
// SASGateway()
//
SASGateway::SASGateway(const string&	myName) :
	GCFTask((State)&SASGateway::connect2SAS, myName),
	itsDPservice (0),
	itsSASservice(0),
	itsTimerPort (0),
	itsPICtreeID (0),
	itsQueryID	 (0)
{
	LOG_DEBUG_STR("SASGateway(" << myName << ")");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("SASGateway"));

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL,	 DP_PROTOCOL_STRINGS);

	// initialize the ports
	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

	itsSASdbname  = globalParameterSet()->getString("SASGateway.OTDBdatabase");
	itsSAShostname= globalParameterSet()->getString("SASGateway.OTDBhostname");
	itsSASservice = new OTDBconnection("paulus", "boskabouter", itsSASdbname, itsSAShostname);
	ASSERTSTR(itsSASservice, "Can't allocate connection to SAS");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate timer");
}

//
// ~SASGateway()
//
SASGateway::~SASGateway()
{
	LOG_DEBUG_STR("~SASGateway()");
	
	if (itsTimerPort) {	
		delete itsTimerPort;
	}
	if (itsDPservice) {
		if (itsQueryID) {
			itsDPservice->cancelQuery(itsQueryID);
			itsQueryID = 0;
		}
		delete itsDPservice;
	}
	if (itsSASservice) {
		delete itsSASservice;
	}
}

//
// connect2SAS(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult SASGateway::connect2SAS(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("connect2SAS:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT:
	case F_TIMER: {
		// Connect to the SAS database
		LOG_INFO(formatString("Trying to connect to SAS database %s@%s", 
						itsSASdbname.c_str(), itsSAShostname.c_str()));
		itsSASservice->connect();
		if (!itsSASservice->isConnected()) {
			LOG_WARN("Connection to SAS failed. Retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			return (status);
		}

		// Get ID of the current PIC tree
		LOG_INFO("Connection to SAS database succesfull, getting info about instrument");
		ClassifConv		CTconv(itsSASservice);
		TreeTypeConv	TTconv(itsSASservice);
		vector<OTDBtree>	treeList =	
						itsSASservice->getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
		ASSERTSTR(treeList.size() == 1, "Expected 1 hardware tree in SAS, database error=" 
										<< itsSASservice->errorMsg());
		itsPICtreeID = treeList[0].treeID();
		LOG_INFO(formatString("Using PICtree %d (%s)", 
							  itsPICtreeID, to_simple_string(treeList[0].starttime).c_str()));

		// SAS part seems OK, continue with PVSS part.
		TRAN(SASGateway::connect2PVSS);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return (status);
}

//
// connect2PVSS(event, port)
//
// Try to open our listener socket
//
GCFEvent::TResult SASGateway::connect2PVSS(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("connect2PVSS:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER: {
		// take subscribtion on *.state
		LOG_DEBUG("Taking subscription on all state fields");
		PVSSresult	result = itsDPservice->query("'LOFAR_PIC_*.status.state'", "");
		if (result != PVSS::SA_NO_ERROR) {
			LOG_ERROR ("Taking subscription on PVSS-states failed, retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		// wait for DP event
		LOG_DEBUG ("Waiting for subscription answer");
	}
	break;

	case DP_QUERY_SUBSCRIBED: {
		DPQuerySubscribedEvent	answer(event);
		if (answer.result != PVSS::SA_NO_ERROR) {
			LOG_ERROR_STR ("Taking subscription on PVSS-states failed (" << answer.result  <<
						   "), retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		itsQueryID = answer.QryID;
		LOG_INFO_STR("Subscription on state fields from PVSS successful(" << itsQueryID  <<
					 "), going to operational mode");
		itsTimerPort->cancelAllTimers();
		TRAN(SASGateway::operational);
	}
	break;

	case DP_QUERY_CHANGED:
		// don't expect this event here right now, but you never know.
		_handleQueryEvent(event);
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
GCFEvent::TResult SASGateway::operational(GCFEvent&			event, 
												GCFPortInterface&	port)
{
	LOG_DEBUG_STR("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		break;

	case DP_QUERY_CHANGED:
		// don't expect this event here right now, but you never know.
		_handleQueryEvent(event);
		break;

	// TODO: add DP lost subscription events and so on.


	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


//
// _handleQueryEvent(event, port)
//
void SASGateway::_handleQueryEvent(GCFEvent&	event)
{
	DPQueryChangedEvent		DPevent(event);
	
	if (DPevent.result != PVSS::SA_NO_ERROR) {
		LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for query " << itsQueryID);
		return;
	}

	int		nrDPs = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
	GCFPVDynArr*	DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
	GCFPVDynArr*	DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
	GCFPVDynArr*	DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);
	TreeValue		tv(itsSASservice, itsPICtreeID);
	for (int	idx = 0; idx < nrDPs; ++idx) {
		// show operator what we are doing
		string	nameStr(PVSS2SASname(DPnames->getValue() [idx]->getValueAsString()));
		string	valStr (DPvalues->getValue()[idx]->getValueAsString());
		string	timeStr(DPtimes->getValue() [idx]->getValueAsString());
		LOG_INFO_STR(nameStr << " = " << valStr << " @ " << timeStr);

		tv.addKVT(nameStr, valStr, time_from_string(timeStr));
	} // for
}



  } // namespace RTDBDaemons
 } // namespace GCF
} // namespace LOFAR
