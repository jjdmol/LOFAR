//#  tControllerDefines.cc
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <APL/APLCommon/ControllerDefines.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	uint16		cntlrType(CNTLRTYPE_BEAMCTRL);
	uint16		instanceNr(8);
	uint32		obsNr(123);

	string	cntlrName = controllerName(cntlrType, instanceNr, obsNr);
	LOG_INFO_STR("Controllername = " << cntlrName);
	ASSERTSTR (cntlrName == "BeamControl[8]{123}", 
			"Expecting cntlrName 'BeamControl[8]{123}' in stead of " << cntlrName);

	string	sharedName = sharedControllerName(cntlrName);
	LOG_INFO_STR("SharedName = " << sharedName);
	ASSERTSTR (sharedName == "BeamControl{123}", 
			"Expecting sharedName 'BeamControl{123}' in stead of " << sharedName);

	string	execName = getExecutable(cntlrType);
	LOG_INFO_STR("Executable = " << execName);
	ASSERTSTR (execName == "BeamControl",
			"Expecting executable 'BeamControl' in stead of " << execName);

	ASSERTSTR (isSharedController(CNTLRTYPE_GROUPCTRL), 
			"Expected group controller to be shared.");

	uint32	retrievedObsNr = getObservationNr(cntlrName);
	LOG_INFO_STR("ObservationNr = " << retrievedObsNr);
	ASSERTSTR (retrievedObsNr == obsNr, 
			"Expected observationNr " << obsNr << " in stead of " << retrievedObsNr);

	uint32	retrievedInstanceNr = getInstanceNr(cntlrName);
	LOG_INFO_STR("InstanceNr = " << retrievedInstanceNr);
	ASSERTSTR (retrievedInstanceNr == instanceNr, 
			"Expected instanceNr " << instanceNr << " in stead of " 
			<< retrievedInstanceNr);

	uint16	retrievedCntlrType = getControllerType(cntlrName);
	LOG_INFO_STR("ControllerType = " << cntlrType);
	ASSERTSTR (retrievedCntlrType == cntlrType, 
			"Expected controllerType " << cntlrType << " in stead of " 
			<< retrievedCntlrType);

	LOG_INFO_STR("ObservationNr of shared name= " << getObservationNr(sharedName));
	LOG_INFO_STR("InstanceNr of shared name   = " << getInstanceNr(sharedName));
	LOG_INFO_STR("CntlrType of shared name    = " << getControllerType(sharedName));
	LOG_INFO_STR("Sharedname of shared name   = " << sharedControllerName(sharedName));

	return (0);
}

