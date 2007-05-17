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
#include "Controller_Protocol.ph"

namespace LOFAR {
  namespace APLCommon {

typedef struct stateSignal {
	CTState::CTstateNr	state;
	uint16				signal;
	char*				name;
} stateSignal_t;

stateSignal_t	stateSignalTable[] = {
	{	CTState::NOSTATE,	0x00,				"Unknown" 	},
	{	CTState::CREATED,	CONTROL_STARTED,	"Created"	},
	{	CTState::CONNECT,	CONTROL_CONNECT,	"Connecting"},
	{	CTState::CONNECTED,	CONTROL_CONNECTED,	"Connected"	},
	{	CTState::RESYNC,	CONTROL_RESYNC,		"Resyncing"	},
	{	CTState::RESYNCED,	CONTROL_RESYNCED,	"Resynced"	},
	{	CTState::SCHEDULE,	CONTROL_SCHEDULE,	"Schedule"	},
	{	CTState::SCHEDULED,	CONTROL_SCHEDULED,	"Scheduled"	},
	{	CTState::CLAIM,		CONTROL_CLAIM,		"Claiming"	},
	{	CTState::CLAIMED,	CONTROL_CLAIMED,	"Claimed"	},
	{	CTState::PREPARE,	CONTROL_PREPARE,	"Preparing"	},
	{	CTState::PREPARED,	CONTROL_PREPARED,	"Prepared"	},
	{	CTState::RESUME,	CONTROL_RESUME,		"Activating"},
	{	CTState::RESUMED,	CONTROL_RESUMED,	"Active"	},
	{	CTState::SUSPEND,	CONTROL_SUSPEND,	"Suspending"},
	{	CTState::SUSPENDED,	CONTROL_SUSPENDED,	"Suspended"	},
	{	CTState::RELEASE,	CONTROL_RELEASE,	"Releasing"	},
	{	CTState::RELEASED,	CONTROL_RELEASED,	"Released"	},
	{	CTState::QUIT,		CONTROL_QUIT,		"Quiting"	},
	{	CTState::QUITED,	CONTROL_QUITED,		"Quited"	}
};

//
// CTState()
//
CTState::CTState()
{
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
	return ((aStateNr < LAST_STATE) ?  stateSignalTable[aStateNr].name : "NoConnection");
}

//
// value(name)
//
uint16	CTState::value(const string&		aStateName) const
{
	uint16	i = NOSTATE;
	while (i < LAST_STATE) {
		if (stateSignalTable[i].name == aStateName) {
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

//
// signal(CTstateNr)
//
uint16	CTState::signal(CTstateNr	aStateNr) const
{
	ASSERTSTR((aStateNr >= NOSTATE) && (aStateNr < LAST_STATE), 
								aStateNr << " is not a valid CTState");

	return (stateSignalTable[aStateNr].signal);
}

//
// signal2stateNr(signal)
//
CTState::CTstateNr	CTState::signal2stateNr  (uint16	someSignal)   const
{
	uint16	i = NOSTATE;
	while (i < LAST_STATE) {
		if (stateSignalTable[i].signal == someSignal) {
			return (stateSignalTable[i].state);
		}
		i++;
	}

	ASSERTSTR(false, someSignal << " is not a supported signal");
}

//
// stateAck(state)
//
// Return the 'ack' state of the given state.
//
CTState::CTstateNr	CTState::stateAck(CTstateNr		aStateNr) const
{
	ASSERTSTR((aStateNr >= NOSTATE) && (aStateNr < LAST_STATE), 
								aStateNr << " is not a valid CTState");

	switch (aStateNr) {
	case CONNECT:
	case RESYNC:
	case SCHEDULE:
	case CLAIM:
	case PREPARE:
	case RESUME:
	case SUSPEND:
	case RELEASE:
	case QUIT:
		return (stateNr(value(aStateNr)+1));
	default:
		return (aStateNr);
	}
}

  } // namespace APL
} // namespace LOFAR
