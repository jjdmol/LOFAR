//#  PR_Shell.cc: ProcessRule based on shell scripts
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
#include<Common/LofarLogger.h>
#include<ACC/PR_Shell.h>

namespace LOFAR {
  namespace ACC {

PR_Shell::PR_Shell(const string&  aNodeName,
				   const string&  aProcName,
				   const string&  aParamFile) :
	ProcRule(aNodeName, aProcName, aParamFile)
{
	// TODO: do something with itsNodeName when ruling the process.
	itsStartCmd = formatString("./startAP.sh %s ./APTest %s", 
								aProcName.c_str(),
								aParamFile.c_str());
	itsStopCmd  = formatString("./stopAP.sh %s", 
								aProcName.c_str());
}

bool PR_Shell::start()
{
	if (itsIsStarted) {
		LOG_TRACE_OBJ_STR("PR_Shell:" << itsProcName << " is already started");
		return (true);
	}

	LOG_TRACE_OBJ_STR ("PR_Shell:start " << itsProcName);

	int32 result = system (itsStartCmd.c_str());

	if (result == 0) {
		itsIsStarted = true;
	}

	return (itsIsStarted);
}

bool PR_Shell::stop()
{
	// Note: always execute the stop command because it may also cleanup
	// some mess the process left behind.
	LOG_TRACE_OBJ_STR ("PR_Shell:stop " << itsProcName);

	int32 result = system (itsStopCmd.c_str());

	if (result == 0) {
		itsIsStarted = false;
	}

	return (!itsIsStarted);
}

  } // namespace ACC
} // namespace LOFAR
