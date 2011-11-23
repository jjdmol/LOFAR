//#  CTState.h: Converter class for handling ConTroller states
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

#ifndef APL_CTSTATE_H
#define APL_CTSTATE_H

// \file
// Converter class for handling ConTroller states.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APLCommon {

// \addtogroup package
// @{


// class_description
// ...
class CTState
{
public:
	CTState();
	~CTState();

	// define enumeration for all states of an LogicalDevice.
	typedef enum {
		ANYSTATE = -1,
		NOSTATE = 0,
		CREATED,
		CONNECT,		// child to parent
		CONNECTED,
		RESYNC,			// child to parent
		RESYNCED,
		SCHEDULE,
		SCHEDULED,
		CLAIM,
		CLAIMED,
		PREPARE,
		PREPARED,
		RESUME,
		RESUMED,
		SUSPEND,
		SUSPENDED,
		RELEASE,
		RELEASED,
		QUIT,
		QUITED,
		LAST_STATE
	} CTstateNr;

	// conversion routines
	string		name    		(uint16			aStateNr)   const;
	uint16		value   		(const string&	aStateName) const;
	uint16		value   		(CTstateNr		aStateNr)   const;
	CTstateNr	stateNr 		(uint16			someNr)     const;
	uint16		signal  		(CTstateNr		aStateNr)   const;
	CTstateNr	stateAck		(CTstateNr		aStateNr)	const;
	CTstateNr	signal2stateNr  (uint16			someSignal) const;

private:
	// Copying is not allowed
	CTState(const CTState&	that);
	CTState& operator=(const CTState& that);

	//# --- Datamembers ---
};

// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
