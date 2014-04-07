//#  tOutOfBand.cc: Test out of band chains
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
#include <Common/LofarTypes.h>
#include <APL/APLCommon/CTState.h>
#include "Controller_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::APLCommon;

typedef struct stateFlow_t {
	uint16				signal;
	CTState::CTstateNr	currentState;
	CTState::CTstateNr	requestedState;
} stateFlow;

static	stateFlow	stateFlowTable[] = {
//		received signal			expected in state		requested state
//		------------------------------------------------------------------
	{	CONTROL_CONNECTED,		CTState::CONNECT,		CTState::CONNECTED	},
	{	CONTROL_CLAIM,			CTState::CONNECTED,		CTState::CLAIMED	},
	{	CONTROL_CLAIMED,		CTState::CLAIM,			CTState::CLAIMED	},
	{	CONTROL_PREPARE,		CTState::CLAIMED,		CTState::PREPARED	},
	{	CONTROL_PREPARED,		CTState::PREPARE,		CTState::PREPARED	},
	{	CONTROL_RESUME,			CTState::PREPARED,		CTState::RESUMED	},
	{	CONTROL_RESUME,			CTState::SUSPENDED,		CTState::RESUMED	},
	{	CONTROL_RESUMED,		CTState::RESUME,		CTState::RESUMED	},
	{	CONTROL_SUSPEND,		CTState::RESUMED,		CTState::SUSPENDED	},
	{	CONTROL_SUSPEND,		CTState::PREPARED,		CTState::SUSPENDED	},
	{	CONTROL_SUSPENDED,		CTState::SUSPEND,		CTState::SUSPENDED	},
	{	CONTROL_RELEASE,		CTState::CLAIMED,		CTState::RELEASED	},
	{	CONTROL_RELEASE,		CTState::PREPARED,		CTState::RELEASED	},
	{	CONTROL_RELEASE,		CTState::SUSPENDED,		CTState::RELEASED	},
	{	CONTROL_RELEASED,		CTState::RELEASE,		CTState::RELEASED	},
	{	CONTROL_QUIT,			CTState::CONNECTED,		CTState::QUITED		},
	{	CONTROL_QUIT,			CTState::RELEASED,		CTState::QUITED		},
//	{	CONTROL_QUIT,			CTState::ANYSTATE,		CTState::QUITED		},
	{	CONTROL_QUITED,			CTState::QUIT,			CTState::QUITED		},
	{	CONTROL_RESYNCED,		CTState::ANYSTATE,		CTState::ANYSTATE	},
	{	CONTROL_SCHEDULE,		CTState::ANYSTATE,		CTState::ANYSTATE	},
	{	0x00,					CTState::NOSTATE,		CTState::NOSTATE	}
};

//
// getNextState(cur, requested)
//
// Returns the state that must be realized. When a signal is received 'out of band'
// of the 'normal' sequence the missing state is returned.
//
CTState::CTstateNr getNextState(CTState::CTstateNr		theCurrentState,
							    CTState::CTstateNr		theRequestedState)
{
	CTState		cts;
//	LOG_DEBUG_STR("getNextState(" << 
//						cts.name(theCurrentState) << ", " << 
//						cts.name(theRequestedState) << ")");

	if (theCurrentState == theRequestedState) {
		return (theRequestedState);
	}

	// look if signal is inband
	uint32	i = 0;
	while (stateFlowTable[i].signal) {
		if ((stateFlowTable[i].requestedState == theRequestedState) &&
			(stateFlowTable[i].currentState == theCurrentState)) {
			// yes, requested state is allowed in the current state
			return(stateFlowTable[i].requestedState);
		}
		i++;
	}

	// signal is not an inband signal, try to find a path the leads to the req. state
	CTState::CTstateNr	requestedState = theRequestedState;
	CTState::CTstateNr	currentState   = theCurrentState;
	i = 0;
	for (;;) {
//		LOG_DEBUG_STR("Check " << cts.name(stateFlowTable[i].currentState) << 
//						" to " << cts.name(stateFlowTable[i].requestedState));
		// find matching requested state
		if (stateFlowTable[i].requestedState == requestedState) {
			// does (moved) currentState match state of table?
			if (stateFlowTable[i].currentState == currentState) {
				LOG_INFO_STR("State change from " << cts.name(theCurrentState) <<
						" to " << cts.name(theRequestedState) << 
						" is out of band. First going to state " << 
						cts.name(stateFlowTable[i].requestedState));
				return (stateFlowTable[i].requestedState);
			}

			// requested state may be reached from anystate?
			if (stateFlowTable[i].currentState == CTState::ANYSTATE) {
				return (stateFlowTable[i].requestedState);
			}

			// does this state lay between currentstate of parent and requested?
			if (stateFlowTable[i].currentState > currentState) {
				// adopt this step and try to resolve it
				requestedState = stateFlowTable[i].currentState;
				i = 0;
				continue;
			}
		}

		i++;
		if (!stateFlowTable[i].signal) {
			// no matching route found. Just return requested state and 
			// hope for the best.
			LOG_WARN_STR("Not supported state change from " << 
						cts.name(theCurrentState) << " to " << 
						cts.name(theRequestedState) << ". Hope it will work!");
			return (theRequestedState);
		}
	}
}

int main (int	argc, char*		argv[]) {

	INIT_LOGGER(basename(argv[0]));

	CTState		cts;

	LOG_DEBUG_STR("connected-> prepared : " << cts.name(getNextState(CTState::CONNECTED, 
																   CTState::PREPARED)));
	LOG_DEBUG_STR("connected-> released : " << cts.name(getNextState(CTState::CONNECTED, 
																   CTState::RELEASED)));
	LOG_DEBUG_STR("claimed  -> prepared : " << cts.name(getNextState(CTState::CLAIMED, 
																   CTState::PREPARED)));
	LOG_DEBUG_STR("claimed  -> resumed  : " << cts.name(getNextState(CTState::CLAIMED, 
																   CTState::RESUMED)));
	LOG_DEBUG_STR("claimed  -> suspended: " << cts.name(getNextState(CTState::CLAIMED, 
																   CTState::SUSPENDED)));
	LOG_DEBUG_STR("claimed  -> released : " << cts.name(getNextState(CTState::CLAIMED, 
																   CTState::RELEASED)));
	LOG_DEBUG_STR("prepared -> prepared : " << cts.name(getNextState(CTState::PREPARED, 
																   CTState::PREPARED)));
	LOG_DEBUG_STR("prepared -> resumed  : " << cts.name(getNextState(CTState::PREPARED, 
																   CTState::RESUMED)));
	LOG_DEBUG_STR("prepared -> suspended: " << cts.name(getNextState(CTState::PREPARED, 
																   CTState::SUSPENDED)));
	LOG_DEBUG_STR("prepared -> released : " << cts.name(getNextState(CTState::PREPARED, 
																   CTState::RELEASED)));
	LOG_DEBUG_STR("suspended-> prepared : " << cts.name(getNextState(CTState::SUSPENDED, 
																   CTState::PREPARED)));
	LOG_DEBUG_STR("resumed  -> released : " << cts.name(getNextState(CTState::RESUMED, 
																   CTState::RELEASED)));
	LOG_DEBUG_STR("connected-> quited   : " << cts.name(getNextState(CTState::CONNECTED, 
																   CTState::QUITED)));
	LOG_DEBUG_STR("claimed  -> quited   : " << cts.name(getNextState(CTState::CLAIMED, 
																   CTState::QUITED)));
	LOG_DEBUG_STR("prepared -> quited   : " << cts.name(getNextState(CTState::PREPARED, 
																   CTState::QUITED)));
	LOG_DEBUG_STR("resumed  -> quited   : " << cts.name(getNextState(CTState::RESUMED, 
																   CTState::QUITED)));
	LOG_DEBUG_STR("suspended-> quited   : " << cts.name(getNextState(CTState::SUSPENDED, 
																   CTState::QUITED)));


	return (0);
}
