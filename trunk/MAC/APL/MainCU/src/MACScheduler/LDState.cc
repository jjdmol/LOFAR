//#  LDState.cc: one_line_description
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
#include <LDState.h>

namespace LOFAR {
  namespace APLCommon {

//
// LDState()
//
LDState::LDState()
{
	itsStates.resize(LAST_STATE);
	itsStates[NOSTATE] 				= "Unknown";
	itsStates[CONNECT]				= "Connecting";
	itsStates[CONNECTED]			= "Connected";
	itsStates[CLAIM]				= "Claiming";
	itsStates[CLAIMED]				= "Claimed";
	itsStates[PREPARE]				= "Preparing";
	itsStates[PREPARED]				= "Prepared";
	itsStates[ACTIVE]				= "Active";
	itsStates[SUSPEND]				= "Suspending";
	itsStates[SUSPENDED]			= "Suspended";
	itsStates[RELEASE]				= "Releasing";
	itsStates[RELEASED]				= "Released";
	itsStates[FINISH]				= "Finishing";
	itsStates[FINISHED]				= "Finished";
}

//
// ~LDState()
//
LDState::~LDState()
{
}

//
// name(statenr)
//
string	LDState::name(uint16			aStateNr)
{ 
	return (((aStateNr >= NOSTATE) && (aStateNr < LAST_STATE)) ?
											itsStates[aStateNr] : "");
}

//
// value(name)
//
uint16	LDState::value(const string&		aStateName)
{
	uint16	i = NOSTATE;
	while (i < LAST_STATE) {
		if (itsStates[i] == aStateName) {
			return (i);
		}
		i++;
	}

	ASSERTSTR(false, aStateName << " is not a valid LDState");
}

//
// value(LDstateNr)
//
uint16	LDState::value(LDstateNr		aStateNr)
{
	ASSERTSTR((aStateNr >= NOSTATE) && (aStateNr < LAST_STATE), 
								aStateNr << " is not a valid LDState");

	return ((uint16) aStateNr);
}

  } // namespace APL
} // namespace LOFAR
