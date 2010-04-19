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
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/DPservice.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/RTDBCommon/RTDButilities.h>

namespace LOFAR {
  using namespace GCF::TM;
  using namespace GCF::PVSS;
  using namespace GCF::RTDB;
  namespace APL {
    namespace RTDBCommon {

typedef struct RTDBObjState {
	uint32		RTDBvalue;
	char*		name;
} RTDBobjState_t;

RTDBobjState_t	objStateTable[] = {
	{	 0,	"'Off'"				},
	{	10,	"'Operational'"		},
	{	20,	"'Maintenance'"		},
	{	30,	"'Test'"			},
	{	46,	"'Suspicious'"		},
	{	56,	"'Broken'"			},
	{	60,	"'Not available'"	}
};

// Every softwaremodule or hardwarecomponent has a color on the Navigator screens
// that represents the state the object/module is in. With this function an object
// can be given a new state.
bool setObjectState(const string&	who,
					const string&	objectName,
				    uint32			newStateIndex,
					bool			force)
{
	// check newState value
	if (newStateIndex >= RTDB_OBJ_STATE_NR_OF_VALUES) {
		LOG_ERROR_STR(newStateIndex << " is not a legal objectStateIndex, " << objectName 
						<< " will be left unchanged");
		return (false);
	}

	// the DP we must write to has three elements. Make a vector of the names and the new
	// values and write them to the database at once.
	DPservice			aDPservice(0);
	vector<string>		fields;
	vector<GCFPValue*>	values;
	fields.push_back("DPName");
	fields.push_back("stateNr");
	fields.push_back("message");
	fields.push_back("force");
	values.push_back(new GCFPVString(objectName+".status.state"));
	values.push_back(new GCFPVInteger(objStateTable[newStateIndex].RTDBvalue));
	values.push_back(new GCFPVString(who));
	values.push_back(new GCFPVBool(force));

	LOG_DEBUG_STR(who << " is setting " << objectName << " to " << objStateTable[newStateIndex].name);

	PVSSresult	result = aDPservice.setValue("__navObjectState", fields, values, 0.0, false);
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR("Call to PVSS for setObjectState returned: " << result);
	}
	return (result == SA_NO_ERROR);
}

//
// objectStateIndex2Value(stateIndex)
//
int32 objectStateIndex2Value(uint32	stateIndex)
{
	if (stateIndex >= RTDB_OBJ_STATE_NR_OF_VALUES) {
		return (-1);
	}
	return (objStateTable[stateIndex].RTDBvalue);
}



    } // namespace RTDBCommon
  } // namespace APL
} // namespace LOFAR
