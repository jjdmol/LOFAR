//#  LogicalDeviceState.cc: one_line_description
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
#include <LogicalDeviceState.h>

namespace LOFAR {
  namespace APLCommon {

//
// LogicalDeviceState()
//
LogicalDeviceState::LogicalDeviceState()
{
	itsStates.resize(LAST_STATE);
	itsStates[UNKNOWN] 				= "Unknown";
	itsStates[CONNECT]				= "Connecting";
	itsStates[CONNECTED]			= "Connected";
	itsStates[SCHEDULE]				= "Scheduling";
	itsStates[SCHEDULED]			= "Scheduled";
	itsStates[CANCEL_SCHEDULE]		= "Canceling Schedule";
	itsStates[SCHEDULE_CANCELLED]	= "Schedule Cancelled";
	itsStates[CLAIM]				= "Claiming";
	itsStates[CLAIMED]				= "Claimed";
	itsStates[PREPARE]				= "Preparing";
	itsStates[PREPARED]				= "Prepared";
	itsStates[RESUME]				= "Resuming";
	itsStates[RESUMED]				= "Resumed";
	itsStates[SUSPEND]				= "Suspending";
	itsStates[SUSPENDED]			= "Suspended";
	itsStates[RELEASE]				= "Releasing";
	itsStates[RELEASED]				= "Released";
	itsStates[FINISH]				= "Finishing";
	itsStates[FINISHED]				= "Finished";
}

//
// ~LogicalDeviceState()
//
LogicalDeviceState::~LogicalDeviceState()
{
}

string	LogicalDeviceState::name(uint16			aStateNr)
{ 
	return (((aStateNr >= UNKNOWN) && (aStateNr < LAST_STATE)) ?
											itsStates[aStateNr] : "");
}

uint16	LogicalDeviceState::value(const string&		aStateName)
{
	uint16	i = UNKNOWN;
	while (i < LAST_STATE) {
		if (itsStates[i] == aStateName) {
			return (i);
		}
		i++;
	}

	ASSERTSTR(false, aStateName << " is not a valid LogicalDeviceState");
}

  } // namespace APL
} // namespace LOFAR
