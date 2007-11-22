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


// The states each softwaremodule and hardwarecomponent can have.
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
bool	setObjectState(const string&	ObjectName,
					   uint32			newState);



// @}
    } // namespace RTDBCommon
  } // namespace APL
} // namespace LOFAR

#endif
