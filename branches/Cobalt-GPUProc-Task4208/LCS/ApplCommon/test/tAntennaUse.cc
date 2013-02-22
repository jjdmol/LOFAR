//#  tAntennaUse.cc
//#
//#  Copyright (C) 2008
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
//#  $Id: tAntennaSet.cc 15222 2010-03-15 14:27:41Z loose $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <blitz/array.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_vector.h>
#include <ApplCommon/AntennaField.h>
#include <ApplCommon/AntennaSets.h>

using namespace blitz;
using namespace LOFAR;

int main (int	argc, char* argv[]) 
{
        (void)argc;

	INIT_VAR_LOGGER(argv[0], argv[0]);

	AntennaField	theAF("tAntennaUse.in_1");	// read the AntennaField.conf file into memory
	AntennaSets		theAS("tAntennaUse.in_2");	// read the AntennaSets.conf file into memory

	// Show the names of the sets.
	LOG_DEBUG_STR("The AntennaSets.conf file containes the following sets:");
	vector<string>	theNames = theAS.antennaSetList();
	for (uint idx = 0; idx < theNames.size(); idx++) {
		LOG_DEBUG_STR(idx << " : " << theNames[idx] << " : " << (theAS.usesLBAfield(theNames[idx], 0) ? "LBA" : "HBA")
						<< " on field " << theAS.antennaField(theNames[idx],0));
	}

	// show all configurations
	for (uint idx = 0; idx < theNames.size(); idx++) {
		string	antennaSet(theNames[idx]);
		string	fieldName(theAS.antennaField(antennaSet));
		LOG_DEBUG_STR("********** " << theNames[idx] << " on " << fieldName << " **********");
		LOG_DEBUG_STR("RCUs EUROPE:" << theAS.RCUinputs(antennaSet, 2));
		LOG_DEBUG_STR("RCUs REMOTE:" << theAS.RCUinputs(antennaSet, 1));
		LOG_DEBUG_STR("RCUs CORE  :" << theAS.RCUinputs(antennaSet, 0));

		blitz::Array<double,2>	rcuPos = theAF.RCUPos(fieldName);
		vector<int16>	posIndex = theAS.positionIndex(antennaSet, 2);
		LOG_DEBUG_STR("EUROPE");
		int		nrPrinted(0);
		for (int rcu = 0; rcu < MAX_RCUS && nrPrinted < 6; rcu++) {
			if ((theAS.RCUallocation(antennaSet, 2)).test(rcu)) {
				LOG_DEBUG(formatString("RCU[%d]: %9.4f %9.4f %9.4f", rcu, 
					rcuPos((int)posIndex[rcu],0), rcuPos((int)posIndex[rcu],1), rcuPos((int)posIndex[rcu],2)));
				nrPrinted++;
			}
		}
		
		rcuPos = theAF.RCUPos(fieldName);
		posIndex = theAS.positionIndex(antennaSet, 1);
		nrPrinted = 0;
		LOG_DEBUG_STR("REMOTE");
		for (int rcu = 0; rcu < MAX_RCUS && nrPrinted < 6; rcu++) {
			if ((theAS.RCUallocation(antennaSet, 1)).test(rcu)) {
				LOG_DEBUG(formatString("RCU[%d]: %9.4f %9.4f %9.4f", rcu, 
					rcuPos((int)posIndex[rcu],0), rcuPos((int)posIndex[rcu],1), rcuPos((int)posIndex[rcu],2)));
				nrPrinted++;
			}
		}
		
		rcuPos = theAF.RCUPos(fieldName);
		posIndex = theAS.positionIndex(antennaSet, 0);
		nrPrinted = 0;
		LOG_DEBUG_STR("CORE");
		for (int rcu = 0; rcu < MAX_RCUS && nrPrinted < 6; rcu++) {
			if ((theAS.RCUallocation(antennaSet, 0)).test(rcu)) {
				LOG_DEBUG(formatString("RCU[%d]: %9.4f %9.4f %9.4f", rcu, 
					rcuPos((int)posIndex[rcu],0), rcuPos((int)posIndex[rcu],1), rcuPos((int)posIndex[rcu],2)));
				nrPrinted++;
			}
		}
		
	}
	return (0);
}

