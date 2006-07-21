//#  CTState.cc: Converter class for handling ConTroller states
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
#include <APL/APLCommon/CTState.h>

namespace LOFAR {
  namespace APLCommon {

//
// CTState()
//
CTState::CTState()
{
	itsStates.resize(LAST_STATE);
	itsStates[NOSTATE] 				= "Unknown";
	itsStates[CREATED] 				= "Created";
	itsStates[CONNECT]				= "Connecting";
	itsStates[CONNECTED]			= "Connected";
	itsStates[RESYNC]				= "Resyncing";
	itsStates[RESYNCED]				= "Resynced";
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
// ~CTState()
//
CTState::~CTState()
{
}

//
// name(statenr)
//
string	CTState::name(uint16			aStateNr) const
{ 
	return ((aStateNr < LAST_STATE) ?  itsStates[aStateNr] : "NoConnection");
}

//
// value(name)
//
uint16	CTState::value(const string&		aStateName) const
{
	uint16	i = NOSTATE;
	while (i < LAST_STATE) {
		if (itsStates[i] == aStateName) {
			return (i);
		}
		i++;
	}

	ASSERTSTR(false, aStateName << " is not a valid CTState");
}

//
// value(CTstateNr)
//
uint16	CTState::value(CTstateNr		aStateNr) const
{
	ASSERTSTR((aStateNr >= NOSTATE) && (aStateNr < LAST_STATE), 
								aStateNr << " is not a valid CTState");

	return ((uint16) aStateNr);
}

//
// stateNr(uint16)
//
CTState::CTstateNr	CTState::stateNr(uint16		someNr) const
{
	CTState::CTstateNr		maybeAState = (CTState::CTstateNr) someNr;
	ASSERTSTR((maybeAState >= NOSTATE) && (maybeAState < LAST_STATE), 
								someNr << " is not a valid CTState");

	return (maybeAState);


}

  } // namespace APL
} // namespace LOFAR
