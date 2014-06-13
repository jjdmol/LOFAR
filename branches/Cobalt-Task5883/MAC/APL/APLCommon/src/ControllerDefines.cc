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
#include <ApplCommon/LofarDirs.h>
#include <APL/APLCommon/ControllerDefines.h>
#include "Controller_Protocol.ph"

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  namespace APLCommon {

typedef struct cntlrDefinition {
	const char*	cntlrName;
	const char*	parsetName;
	bool		shared;
} cntlrDefinition_t;

static cntlrDefinition_t controllerTable[] = {
//		executable				parsetNode				shared
//--------------------------------------------------------------------
	{	"",						"",						false	},
	{	"MACScheduler", 		"MACScheduler",			false	},
	{	"ObservationControl", 	"ObservationControl",	false	},
	{	"OnlineControl", 		"OnlineControl",		false	},
	{	"OfflineControl", 		"OfflineControl",		false	},
	{	"BeamDirectionControl",	"BeamDirControl",		true	},
	{	"RingControl", 			"RingControl",			true	},
	{	"StationControl", 		"StationControl",		true	},
	{	"ClockControl",			"ClockControl",			true	},
	{	"BeamControl", 			"BeamControl",			false	},
	{	"CalibrationControl", 	"CalControl",			false	},
	{	"TBBControl",		 	"TBBControl",			false	},
	{	"PythonControl", 		"PythonControl",		false	},
	{	"TestController", 		"TestControl",			false	},
	{	"",						"",						false	}
};

#if 0
static const char*	modeNameTable[] = {
	"off",
	"operational",
	"maintenance",
	"test",
	"suspicious",
	"broken",
	""
};
#endif

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

//	if (ObservationNr == 0) {		// used when starting up shared controllers
	if (ObservationNr == 0 && isSharedController(cntlrType)) {
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
// getControllerType(controllerName)
//
// Get the controllerType from the controllername.
int32	getControllerType	(const string&	controllerName)
{
	string	cntlrName(controllerName);		// destroyable copy
	rtrim(cntlrName, "[]{}0123456789");		// cut down to executable name
	string::size_type	pos;
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
	case CONTROL_STARTED: {
//		CONTROLStartedEvent	answer;
//		answer.cntlrName = cntlrName;
//		return (port.send(answer) > 0);
	} break;
	case CONTROL_CONNECT:
	case CONTROL_CONNECTED: {
		CONTROLConnectedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_RESYNC:
	case CONTROL_RESYNCED: {
		CONTROLResyncedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_SCHEDULE:
	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_CLAIM:
	case CONTROL_CLAIMED: {
		CONTROLClaimedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_PREPARE:
	case CONTROL_PREPARED: {
		CONTROLPreparedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_RESUME:
	case CONTROL_RESUMED: {
		CONTROLResumedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_SUSPEND:
	case CONTROL_SUSPENDED: {
		CONTROLSuspendedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_RELEASE:
	case CONTROL_RELEASED: {
		CONTROLReleasedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	case CONTROL_QUIT:
	case CONTROL_QUITED: {
		CONTROLQuitedEvent	answer;
		answer.cntlrName = cntlrName;
		answer.result	 = result;
		return (port.send(answer) > 0);
	} break;
	default:
		ASSERTSTR(false, formatString("State %04X is not supported by 'sendControlResult'", signal));
	}

    return (false); // satisfy compiler        
}


  } // namespace APLCommon
} // namespace LOFAR
