//#  ProcRule.cc: one line description
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
#include<ACC/ProcRule.h>

namespace LOFAR {
  namespace ACC {

ProcRule::ProcRule(const string&  aName,
				   const string&  aStartCmd,
				   const string&  aStopCmd,
				   const string&  aNodeName) :
	itsName     (aName),
	itsStartCmd (aStartCmd),
	itsStopCmd  (aStopCmd),
	itsNodeName (aNodeName),
	itsIsStarted(false)
{}

bool ProcRule::start()
{
	if (itsIsStarted) {
		LOG_TRACE_OBJ_STR("ProcRule:" << itsName << " is already started");
		return (true);
	}

	LOG_TRACE_OBJ_STR ("ProcRule:start " << itsName);

	// TODO: do something with itsNodeName when starting the process.
	// Perhaps this should be implemented in ApplController::createPSubsets
	int32 result = system (itsStartCmd.c_str());

	if (result == 0) {
		itsIsStarted = true;
	}

	return (itsIsStarted);
}

bool ProcRule::stop()
{
	// TODO: do something with itsNodeName

//	if (!itsIsStarted) {
//		LOG_TRACE_OBJ_STR("ProcRule:" << itsName << " is already stopped");
//		return (true);
//	}
	// Note: always execute the stop command because it may also cleanup
	// some mess the process left behind.
	LOG_TRACE_OBJ_STR ("ProcRule:stop " << itsName);

	int32 result = system (itsStopCmd.c_str());

	if (result == 0) {
		itsIsStarted = false;
	}

	return (!itsIsStarted);
}


std::ostream& operator<< (std::ostream& os, const ProcRule& aPR)
{
	os << "ProcName: " << aPR.itsName << endl;
	os << "StartCmd: " << aPR.itsStartCmd << endl;
	os << "StopCmd : " << aPR.itsStopCmd  << endl;
	os << "NodeName: " << aPR.itsNodeName << endl;

	return (os);
}

  } // namespace ACC
} // namespace LOFAR
