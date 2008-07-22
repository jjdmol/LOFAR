//#  ControllerDefines.cc: Controller(name) related utilities
//#
//#  Copyright (C) 2006-2008
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
//#
//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>					// rtrim
#include <Common/SystemUtil.h>
#include <APS/ParameterSet.h>					// indexValue
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include "Controller_Protocol.ph"

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  using namespace Deployment;
  namespace APLCommon {

typedef struct cntlrDefinition {
	char*		cntlrName;
	char*		parsetName;
	bool		shared;
} cntlrDefinition_t;

static cntlrDefinition_t controllerTable[] = {
//		executable				parsetNode		shared
//----------------------------------------------------------
	{	"",						"",				false	},
	{	"MACScheduler", 		"MACScheduler",	false	},
	{	"ObservationControl", 	"ObsCtrl",		false	},
	{	"OnlineControl", 		"OnlineCtrl",	false	},
	{	"OfflineControl", 		"OfflineCtrl",	false	},
	{	"BeamDirectionControl",	"BeamDirCtrl",	true	},
	{	"RingControl", 			"RingCtrl",		true	},
	{	"StationControl", 		"StationCtrl",	true	},
	{	"DigitalBoardControl", 	"DigBoardCtrl",	true	},
	{	"BeamControl", 			"BeamCtrl",		false	},
	{	"CalibrationControl", 	"CalCtrl",		false	},
	{	"TBBControl",		 	"TBBCtrl",		false	},
	{	"StationInfraControl", 	"StsInfraCtrl",	true	},
	{	"TestController", 		"TestCtrl",		false	},
	{	"",						"",				false	}
};

static char*	modeNameTable[] = {
	"off",
	"operational",
	"maintenance",
	"test",
	"suspicious",
	"broken",
	""
};

//
// controllerName(type,instance,obsNr,hostname)
//
// Construct a uniq controllername from the controllerType, the instanceNr
// of the controller and the observationID.
// Note: the returned name is always the 'non-shared' name. To get the 'shared'
//		 name passed the result to 'sharedControllerName')
string	controllerName (uint16			cntlrType, 
						uint16			instanceNr, 
						uint32			ObservationNr,
						const string&	hostname)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	string	theHostname(hostname);
	if (theHostname.empty()) {
		theHostname = myHostname(false);
	}

	if (ObservationNr == 0) {		// used when starting up shared controllers
		return (formatString("%s:%s", theHostname.c_str(),
									  controllerTable[cntlrType].cntlrName));
	}

	return (formatString("%s:%s[%d]{%d}", theHostname.c_str(),
										  controllerTable[cntlrType].cntlrName,
										  instanceNr, ObservationNr));
}

//
// parsetNodeName(type)
//
// Convert a controller type to the coresponding node in the OTDB.
string	parsetNodeName (uint16		cntlrType)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].parsetName);
}

//
// sharedControllerName(controllername)
//
// Convert the 'non-shared controllername' to the 'shared controller' name.
string	sharedControllerName (const string&	controllerName)
{
	uint16		cntlrType = getControllerType(controllerName);
	if (!isSharedController(cntlrType)) {
		return (controllerName);
	}

	string	cntlrName(controllerName);			// destroyable copy.
	rtrim(cntlrName, "[]{}0123456789");
	return (cntlrName);
}

//
// getExecutable(type)
//
// Return name of the executable
string	getExecutable (uint16		cntlrType)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].cntlrName);
}

//
// isSharedController(type)
//
// return 'shared' bit of controllertype
bool	isSharedController(uint16		cntlrType) 
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].shared);
}

//
// getObservationNr(controllerName)
//
// Get the ObservationNr from the controllername.
uint32	getObservationNr (const string&	controllerName)
{
	return (ACC::APS::indexValue(controllerName, "{}"));
}

//
// getInstanceNr(controllername)
//
// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	controllerName)
{
	string		cntlrName (controllerName);		// destroyable copy
	rtrim(cntlrName, "{}0123456789");
	return (ACC::APS::indexValue(cntlrName, "[]"));
}

//
// getControllerType(controllerName)
//
// Get the controllerType from the controllername.
int32	getControllerType	(const string&	controllerName)
{
	string	cntlrName(controllerName);		// destroyable copy
	rtrim(cntlrName, "[]{}0123456789");		// cut down to executable name
	uint	pos;
	if ((pos = cntlrName.find(":")) != string::npos) {	// strip off hostname
		cntlrName.erase(0, pos+1);
	}

	// first try on controllername
	uint32	idx = CNTLRTYPE_NO_TYPE + 1;
	while (idx < CNTLRTYPE_NR_TYPES) {
		if (!strcmp (controllerTable[idx].cntlrName, cntlrName.c_str())) {
			return (idx);
		}
		idx++;
	}

	// not found, may user passed parsetNodename
	idx = CNTLRTYPE_NO_TYPE + 1;
	while (idx < CNTLRTYPE_NR_TYPES) {
		if (!strcmp (controllerTable[idx].parsetName, cntlrName.c_str())) {
			return (idx);
		}
		idx++;
	}

	return (CNTLRTYPE_NO_TYPE);
}

//
// observationName(obsID)
//
string observationName(int	obsID)
{
	return (formatString("Observation%d", obsID));
}

//
// observationParset(obsID)
//
string observationParset(int	obsID)
{
	return (formatString("%s/%s", LOFAR_SHARE_LOCATION, observationName(obsID).c_str()));
}

//
// createPropertySetName(propSetMask)
//
//  A PropSetMask may contain the markers:
//	@ring@
//	@arm@
//	@station@
//  @instance@
//	@observation@
//	@cabinet@
//	@subrack@
//	@RSPboard@
//	@TBboard@
//	@rcu@
//
string	createPropertySetName(const string&		propSetMask,
							  const string&		controllerName,
							  const string&		realDPname)
{
	string	psName(propSetMask);		// editable copy
	uint	pos;
	// when name contains @ring@_@station@ cut out this marker and prepend hostname
	// stationname+:  -> LOFAR_ObsSW_@ring@_@station@_CalCtrl_xxx --> CS010:LOFAR_ObsSW_CalCtrl_xxx
	if ((pos = psName.find("@ring@_@station@_")) != string::npos) {
		psName.erase(pos, 17);
		psName = myHostname(false) + ":" + psName;
	}

	if ((pos = psName.find("@ring@")) != string::npos) {
		psName.replace(pos, 6, string("ring")+lexical_cast<string>(stationRingNr()));
	}
	if ((pos = psName.find("@arm@")) != string::npos) {
		psName.replace(pos, 5, string("arm")+lexical_cast<string>(stationArmNr()));
	}
	if ((pos = psName.find("@instance@")) != string::npos) {
		uint16	instanceNr = getInstanceNr(controllerName);
		if (instanceNr) {
			psName.replace(pos, 10, lexical_cast<string>(instanceNr));
		}
		else {
			psName.replace(pos, 10, "");	
		}
	}
	if ((pos = psName.find("LOFAR_ObsSW_@observation@")) != string::npos) {
		psName.replace(pos, 25, realDPname);
	}

	if ((pos = psName.find("@cabinet@")) != string::npos) {
		psName.replace(pos, 9, string("Cabinet%d"));
	}
	if ((pos = psName.find("@subrack@")) != string::npos) {
		psName.replace(pos, 9, string("Subrack%d"));
	}
	if ((pos = psName.find("@RSPBoard@")) != string::npos) {
		psName.replace(pos, 10, string("RSPBoard%d"));
	}
	if ((pos = psName.find("@TBBoard@")) != string::npos) {
		psName.replace(pos, 9, string("TBBoard%d"));
	}
	if ((pos = psName.find("@rcu@")) != string::npos) {
		psName.replace(pos, 5, string("RCU%d"));
	}
		
	return (psName);
}

//
// sendControlResult(port, CONTROLsignal, cntlrName, result)
//
// Construct a message that matches the given signal and send it on the port.
// Supported are:
// CONNECTED, RESYNCED, SCHEDULED, CLAIMED, PREPARED, RESUMED, SUSPENDED, RELEASED
//
bool sendControlResult(GCF::TM::GCFPortInterface&	port,
					   uint16						signal,
					   const string&				cntlrName,
					   uint16						result)
{
	switch (signal) {
	case CONTROL_CONNECT:
	case CONTROL_CONNECTED: {
			CONTROLConnectedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_RESYNC:
	case CONTROL_RESYNCED: {
			CONTROLResyncedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
			port.send(answer);
		}
		break;
	case CONTROL_SCHEDULE:
	case CONTROL_SCHEDULED: {
			CONTROLScheduledEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_CLAIM:
	case CONTROL_CLAIMED: {
			CONTROLClaimedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_PREPARE:
	case CONTROL_PREPARED: {
			CONTROLPreparedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_RESUME:
	case CONTROL_RESUMED: {
			CONTROLResumedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_SUSPEND:
	case CONTROL_SUSPENDED: {
			CONTROLSuspendedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_RELEASE:
	case CONTROL_RELEASED: {
			CONTROLReleasedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	case CONTROL_QUIT:
	case CONTROL_QUITED: {
			CONTROLQuitedEvent	answer;
			answer.cntlrName = cntlrName;
			answer.result	 = result;
			return (port.send(answer) > 0);
		}
		break;
	default:
		ASSERTSTR(false, 
			formatString("State %04X is not supported by 'sendControlResult'", signal));

	}
}


  } // namespace APLCommon
} // namespace LOFAR
