//#  RTDButilities.h: Generic functions for the RTDB-based applications
//#
//#  Copyright (C) 2007
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
//#  $Id: $

#ifndef RTDBCOMMON_RTDBUTILITIES_H
#define RTDBCOMMON_RTDBUTILITIES_H

// Generic functions for communicating with RTDB-based applications.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APL {
    namespace RTDBCommon {

// \addtogroup APL
// @{

// NOTE: All these utilities are based on access with the PVSS database.
// Since the 'main loop' of PVSS is integrated into the GCF framework
// (in PVSSservice to be precise) the functions in this file will only
// work within a GCF framework. When you call them outside a GCF framework
// the function might return a valid return function but the action is never
// performed because the main-loop of PVSS is never called to process the
// requests.


// Indexes for the states each softwaremodule and hardwarecomponent can have.
enum {
	RTDB_OBJ_STATE_OFF = 0,
	RTDB_OBJ_STATE_OPERATIONAL,
	RTDB_OBJ_STATE_MAINTENANCE,
	RTDB_OBJ_STATE_TEST,
	RTDB_OBJ_STATE_SUSPICIOUS,
	RTDB_OBJ_STATE_BROKEN,

	RTDB_OBJ_STATE_NR_OF_VALUES
};

// Every softwaremodule or hardwarecomponent has a color on the Navigator screens
// that represents the state the object/module is in. With this function an object
// can be given a new state.
// @param who	name of the program that changes the value.
// @param objectname	name of the module you like to set the state of.
// @param newState	the new state (use one of the enum values of above).
// @param force	when true any state change is allowed otherwise only state-changes 
//				that 'worsen' the state.
bool	setObjectState(const string&	who,
					   const string&	objectName,
					   uint32			newStateIndex,
					   bool				force = false);

// Converts an objectState-index to the value that is st in the database.
// @param stateIndex	indexvalue (RTDB_OBJ_STATE_...)
// Returns the database value or -1 in case of an illegal index.
int32	objectStateIndex2Value(uint32	stateIndex);




// @}
    } // namespace RTDBCommon
  } // namespace APL
} // namespace LOFAR

#endif
