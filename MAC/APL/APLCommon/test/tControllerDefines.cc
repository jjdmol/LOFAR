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
#include <Common/SystemUtil.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;
using namespace LOFAR::Deployment;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	uint16		cntlrType(CNTLRTYPE_STATIONCTRL);
	uint16		instanceNr(8);
	uint32		obsNr(123);
	string		hostname(myHostname(false));

	string	cntlrName = controllerName(cntlrType, instanceNr, obsNr);
	LOG_INFO_STR("Controllername = " << cntlrName);
	ASSERTSTR (cntlrName == hostname+":StationControl[8]{123}", 
			"Expecting cntlrName '" << hostname << 
			":StationControl[8]{123}' in stead of " << cntlrName);


	string	sharedName = sharedControllerName(cntlrName);
	LOG_INFO_STR("SharedName = " << sharedName);
	ASSERTSTR (sharedName == hostname+":StationControl", 
			"Expecting sharedName '" << hostname << ":StationControl' in stead of " 
			<< sharedName);

	string	execName = getExecutable(cntlrType);
	LOG_INFO_STR("Executable = " << execName);
	ASSERTSTR (execName == "StationControl",
			"Expecting executable 'StationControl' in stead of " << execName);

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


	LOG_INFO_STR("myHostname(short) = " << myHostname(false));
	LOG_INFO_STR("myHostname(long)  = " << myHostname(true));
	LOG_INFO_STR("ringNumber = " << stationRingNr());
	LOG_INFO_STR("armNumber  = " << stationArmNr());
	LOG_INFO_STR("PVSSDBname = " << PVSSDatabaseName());
	LOG_INFO_STR("hostname of CS010  = " << realHostname("CS010"));
	LOG_INFO_STR("hostname of CS010C = " << realHostname("CS010C"));
	LOG_INFO_STR("hostname of CS010T = " << realHostname("CS010T"));
	LOG_INFO_STR("PropSetName(LOFAR_PermSW_@ring@_@station@_DigBoardCtrl@instance@) = " << createPropertySetName("LOFAR_PermSW_@ring@_@station@_DigBoardCtrl@instance@", "DigitalBoardControl"));




	return (0);
}

