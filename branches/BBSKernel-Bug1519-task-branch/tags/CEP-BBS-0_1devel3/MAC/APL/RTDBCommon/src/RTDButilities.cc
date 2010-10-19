//#  RTDButilities.cc: one_line_description
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <GCF/RTDB/DPservice.h>
#include <GCF/GCF_PVTypes.h>
#include <APL/RTDBCommon/RTDButilities.h>

namespace LOFAR {
  using namespace GCF::Common;
  using namespace GCF::PVSS;
  using namespace GCF::RTDB;
  namespace APL {
    namespace RTDBCommon {

typedef struct RTDBObjState {
	uint32		RTDBvalue;
	char*		name;
} RTDBobjState_t;

RTDBobjState_t	objStateTable[] = {
	{	 0,	"Off"			},
	{	10,	"Operational"	},
	{	20,	"Maintenance"	},
	{	30,	"Test"			},
	{	46,	"Suspicious"	},
	{	56,	"Broken"		},
	{	60,	"Not available"	}
};

// Every softwaremodule or hardwarecomponent has a color on the Navigator screens
// that represents the state the object/module is in. With this function an object
// can be given a new state.
bool setObjectState(const string&	ObjectName,
				    uint32			newState)
{
	// check newState value
	if (newState >= RTDB_OBJ_STATE_NR_OF_VALUES) {
		LOG_ERROR_STR(newState << " is not a legal object-state, " << ObjectName 
						<< " will be left unchanged");
		return (false);
	}

	// Construct command and store it in the right place.
	string	command(ObjectName+".state="+toString(objStateTable[newState].RTDBvalue));
	LOG_DEBUG_STR("Setting state:" << command);
	DPservice	aDPservice(0);
	PVSSresult	result;
	result = aDPservice.setValue("__navObjectState", GCFPVString(command), 0.0, false);

	return (result == SA_NO_ERROR);
}


    } // namespace RTDBCommon
  } // namespace APL
} // namespace LOFAR
