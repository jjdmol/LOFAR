//#  LDState.h: one_line_description
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

#ifndef APL_LOGICALDEVICESTATE_H
#define APL_LOGICALDEVICESTATE_H

// \file LDState.h
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_map.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APLCommon {

// \addtogroup package
// @{


// class_description
// ...
class LDState
{
public:
	LDState();
	~LDState();

	// define enumeration for all states of an LogicalDevice.
	typedef enum {
		UNKNOWN = 0,
		CONNECT,
		CONNECTED,
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
		FINISH,
		FINISHED,
		LAST_STATE
	} LDstateNr;

	// conversion routines
	string	name (uint16			aStateNr);
	uint16	value(const string&		aStateName);
	uint16	value(LDstateNr			aStateNr);

	// ... example
	ostream& print (ostream& os) const
	{	return (os); }

private:
	// Copying is not allowed
	LDState(const LDState&	that);
	LDState& operator=(const LDState& that);

	//# --- Datamembers ---
	vector<string>		itsStates;
};

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const LDState& aLDState)
{	
	return (aLDState.print(os));
}


// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
