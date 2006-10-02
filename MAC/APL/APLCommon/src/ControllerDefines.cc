//#  ControllerDefines.cc: Controller(name) related utilities
//#
//#  Copyright (C) 2006
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
#include <GCF/Utils.h>							// myHostname
#include <APS/ParameterSet.h>					// indexValue
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  using namespace Deployment;
  using namespace GCF::Common;
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
	{	"StationControl", 		"StationCtrl",	false	},
	{	"DigitalBoardControl", 	"DigBoardCtrl",	false	},
	{	"BeamControl", 			"BeamCtrl",		false	},
	{	"CalibrationControl", 	"CalCtrl",		true	},
	{	"StationInfraControl", 	"StsInfraCtrl",	true	},
	{	"TestController", 		"TestCtrl",		false	},
	{	"",						"",				false	}
};

// Construct a uniq controllername from the controllerType, the instanceNr
// of the controller and the observationID.
// Note: the returned name is always the 'non-shared' name. To get the 'shared'
//		 name passed the result to 'sharedControllerName')
string	controllerName (uint16		cntlrType, 
						uint16		instanceNr, 
						uint32		ObservationNr)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (formatString("%s[%d]{%d}", controllerTable[cntlrType].cntlrName,
												instanceNr, ObservationNr));
}

// Convert a controller type to the coresponding node in the OTDB.
string	parsetNodeName (uint16		cntlrType)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].parsetName);
}

// Convert the 'non-shared controllername' to the 'shared controller' name.
string	sharedControllerName (const string&	controllerName)
{
	uint16		cntlrType = getControllerType(controllerName);
	if (!isSharedController(cntlrType)) {
		return (controllerName);
	}

	uint32	observationNr = getObservationNr (controllerName);
	return (formatString("%s{%d}", controllerTable[cntlrType].cntlrName,
															observationNr));
}

// Return name of the executable
string	getExecutable (uint16		cntlrType)
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].cntlrName);
}

// return 'shared' bit of controllertype
bool	isSharedController(uint16		cntlrType) 
{
	ASSERTSTR (cntlrType != CNTLRTYPE_NO_TYPE && cntlrType < CNTLRTYPE_NR_TYPES,
			"No controller defined with type: " << cntlrType);

	return (controllerTable[cntlrType].shared);
}

// Get the ObservationNr from the controllername.
uint32	getObservationNr (const string&	controllerName)
{
	return (ACC::APS::indexValue(controllerName, "{}"));
}

// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	controllerName)
{
	string		cntlrName (controllerName);		// destroyable copy
	rtrim(cntlrName, "{}0123456789");
	return (ACC::APS::indexValue(cntlrName, "[]"));
}

// Get the controllerType from the controllername.
int32	getControllerType	(const string&	controllerName)
{
	string	cntlrName(controllerName);		// destroyable copy
	rtrim(cntlrName, "[]{}0123456789");		// cut down to executable name
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
// createPropertySetName(propSetMask)
//
//  A PropSetMask may contain the markers:
//	@ring@
//	@arm@
//	@station@
//  @instance@
//	@observation@
//
string	createPropertySetName(const string&		propSetMask,
							  const string&		controllerName)
{
	string	psName(propSetMask);		// editable copy
	uint	pos;
	// when name contains @station@ cut of everything before and replace it with 
	// stationname+:  -> LOFAR_PIC_@ring@_@station@_CalCtrl_xxx --> CS010:CalCtrl_xxx
	if ((pos = psName.find("@station@_")) != string::npos) {
		psName.replace(0, pos+10, PVSSDatabaseName() + ":");
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
	if ((pos = psName.find("@observation@")) != string::npos) {
		psName.replace(pos, 13, string("Observation") +
								lexical_cast<string>(getObservationNr(controllerName)));
	}
	if ((pos = psName.find("@RSPBoard@")) != string::npos) {
		psName.replace(pos, 10, string("RSPBoard%d"));
	}
		
	return (psName);
}





  } // namespace APLCommon
} // namespace LOFAR



