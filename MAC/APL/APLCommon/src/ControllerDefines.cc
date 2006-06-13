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
#include <APS/ParameterSet.h>					// indexValue
#include <APL/APLCommon/ControllerDefines.h>

namespace LOFAR {
  namespace APLCommon {

typedef struct cntlrDefinition {
	char*		cntlrName;
	bool		shared;
} cntlrDefinition_t;

static cntlrDefinition_t controllerTable[] = {
	{	"",						false	},
	{	"MACScheduler", 		false	},
	{	"ObservationControl", 	false	},
	{	"BeamDirectionControl",	true	},
	{	"RingControl", 			true	},
	{	"StationControl", 		false	},
	{	"DigitalBoardControl", 	false	},
	{	"BeamControl", 			true	},
	{	"CalibrationControl", 	true	},
	{	"StationInfraControl", 	true	},
	{	"",						false	}
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

	return (formatString("%s(%d){%d}", controllerTable[cntlrType].cntlrName,
										ObservationNr, instanceNr));
}

// Convert the 'non-shared controllername' to the 'shared controller' name.
string	sharedControllerName (const string&	controllerName)
{
	string	cName(controllerName);		// destroyable copy
	rtrim(cName, "{0123456789}");
	return (cName);
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
uint32	getObservationNr (const string&	ObservationName)
{
	return (ACC::APS::indexValue(sharedControllerName(ObservationName), "()"));
}

// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	ObservationName)
{
	return (ACC::APS::indexValue(ObservationName, "{}"));
}

// Get the controllerType from the controllername.
int32	getControllerType	(const string&	controllerName)
{
	string	cntlrName(controllerName);		// destroyable copy
	rtrim(cntlrName, "(){}0123456789");		// cut down to executable name
	uint32	idx = CNTLRTYPE_NO_TYPE + 1;
	while (idx < CNTLRTYPE_NR_TYPES) {
		if (!strcmp (controllerTable[idx].cntlrName, cntlrName.c_str())) {
			return (idx);
		}
		idx++;
	}

	return (CNTLRTYPE_NO_TYPE);
}

  } // namespace APLCommon
} // namespace LOFAR



