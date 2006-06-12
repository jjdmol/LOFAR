//#  LogicalDeviceStarter.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <sys/types.h>
#include <unistd.h>
#include <APL/APLCommon/StartDaemon_Protocol.ph>
#include "LogicalDeviceStarter.h"

namespace LOFAR {
  namespace CUDaemons {

//
// LogicalDeviceStarter(executablename)
//
LogicalDeviceStarter::LogicalDeviceStarter(ParameterSet*	aParSet) :
	itsProgramList()
{
	string		typeMask   ("Controller.%d.type");
	string		programMask("Controller.%d.program");
	string		sharedMask ("Controller.%d.shared");
	LDstart_t	startInfo;

	try {
		for (uint32 counter = 1; ; counter++) {
			string	typeLabel	(formatString(typeMask.c_str(),    counter));
			string	programLabel(formatString(programMask.c_str(), counter));
			string	sharedLabel (formatString(sharedMask.c_str(),  counter));
			startInfo.name       = aParSet->getString(typeLabel);
			startInfo.executable = aParSet->getString(programLabel);
			startInfo.shared     = aParSet->getBool  (sharedLabel);
			itsProgramList.push_back(startInfo);
		}
	}
	catch (Exception&	e) {
		// expected throw: when counter > seqnr used in parameterSet.
	}

	ASSERTSTR(itsProgramList.size() > 0, "No definitions of Controllers found "
										 "in the parameterSet.");

	LOG_DEBUG_STR("Found " << itsProgramList.size() << " ControllerTypes");
		
}

//
// ~LogicalDeviceStarter
//
LogicalDeviceStarter::~LogicalDeviceStarter()
{
	itsProgramList.clear();
}

//
// createLogicalDevice(taskname, paramfile)
//
int32 LogicalDeviceStarter::startController(const string&	ldTypeName,
										    const string&	taskname,
										    const string&	parentHost,
										    const string&	parentService)
{
	// search controller type in table.
	uint32		nrDevices = itsProgramList.size();
	uint32		index	  = 0;
	while (index < nrDevices && itsProgramList[index].name != ldTypeName){
		index++;
	}

	// not found? report problem
	if (index >= nrDevices) {
		LOG_DEBUG_STR("No support for starting controller of the "
					  "type " << ldTypeName << ". See config file.");
		return (SD_RESULT_UNSUPPORTED_TYPE);
	}

	// shared device? report success
	if (itsProgramList[index].shared) {
		LOG_DEBUG_STR("Controller of type " << ldTypeName << " is shared, no start.");
		return (SD_RESULT_NO_ERROR);
	}

	// locate program.
	ProgramLocator		PL;
	string	executable = PL.locate(itsProgramList[index].executable);
	if (executable.empty()) {
		LOG_DEBUG_STR("Executable for controller " << ldTypeName << " not found.");
		return (SD_RESULT_PROGRAM_NOT_FOUND);
	}

	// construct system command
	string	startCmd = formatString("./startController.sh %s %s %s %s", 
									executable.c_str(),
									taskname.c_str(),
									parentHost.c_str(),
									parentService.c_str());
	LOG_DEBUG_STR("About to start: " << startCmd);

	int32	result = system (startCmd.c_str());
	LOG_DEBUG_STR ("Result of start = " << result);

	if (result == -1) {
		return (SD_RESULT_START_FAILED);
	}
	
	return (SD_RESULT_NO_ERROR);

}


  } // namespace CUDaemons
} // namespace LOFAR
