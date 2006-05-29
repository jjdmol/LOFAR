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
	itsProgramList(),
	itsError 	  ()
{
	string		typeMask   ("LogicalDevice.%d.type");
	string		programMask("LogicalDevice.%d.program");
	string		sharedMask ("LogicalDevice.%d.shared");
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

	ASSERTSTR(itsProgramList.size() > 0, "No definition of LogicalDevice found "
										 "in the parameterSet.");

	LOG_DEBUG_STR("Found " << itsProgramList.size() << " LogicalDeviceTypes");
		
}

//
// ~LogicalDeviceStarter
//
LogicalDeviceStarter::~LogicalDeviceStarter()
{
}

//
// createLogicalDevice(taskname, paramfile)
//
int32 LogicalDeviceStarter::createLogicalDevice(const string&	ldTypeName,
											    const string&	taskname,
											    const string&	parentHost,
											    const string&	parentService)
{
	uint32		nrDevices = itsProgramList.size();
	uint32		index(0);
	while (index < nrDevices && itsProgramList[index].name.compare(ldTypeName)){
		index++;
	}
	if (index >= nrDevices) {
		LOG_DEBUG_STR("No support for starting Logical Devices of the "
					  "type " << ldTypeName << ". See config file.");
		return (SD_RESULT_UNSUPPORTED_LD);
	}

	pid_t	cid;
	itsError = SD_RESULT_NO_ERROR;

	switch (cid = fork()) {
		case -1:	// error
			itsError = errno;
			LOG_FATAL_STR("fork() for task '" << taskname << 
						  "' failed with errno: " << errno);
			return (SD_RESULT_UNSPECIFIED_ERROR);
			break;

		case 0: {			// child
			sleep (1);		// give kernel and parent some time
			setsid();
			LOG_DEBUG_STR("About to start: " << 
						  itsProgramList[index].executable <<
						  " " << taskname << " " << parentHost << 
						  " " << parentService );

			// Transform into the real program
			int execError = execl(itsProgramList[index].executable.c_str(), 
								  itsProgramList[index].executable.c_str(), 
								  taskname.c_str(), 
								  parentHost.c_str(), 
								  parentService.c_str(), 
								  0L);
			
			LOG_FATAL_STR(getpid() << "Transform to " << 
						  itsProgramList[index].executable << 
						  " failed()!!! Errno=" << execError);
			return (SD_RESULT_LD_NOT_FOUND);
			break;
		}
		default:	// parent
			LOG_DEBUG_STR(getpid() << ": child process created: " << cid);
			return (cid);		// returning cid of child that will die soon.
			break;
	}
}


  } // namespace CUDaemons
} // namespace LOFAR
