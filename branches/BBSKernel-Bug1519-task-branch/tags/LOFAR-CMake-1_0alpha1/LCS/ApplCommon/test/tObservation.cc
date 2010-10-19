//#  tObservation.cc
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/Observation.h>

using namespace LOFAR;

int main (int argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	try {
		ParameterSet parSet1("tObservation.in_parset1");
		Observation  obs1(&parSet1);
		cout << obs1 << endl;

		// add an extra beam
		parSet1.replace("ObsSW.Observation.nrBeams", "2");
		parSet1.add("ObsSW.Observation.Beam[1].angle1", 		"0.23456789");
		parSet1.add("ObsSW.Observation.Beam[1].angle2", 		"0.123456789");
		parSet1.add("ObsSW.Observation.Beam[1].directionType",  "AZEL");
		parSet1.add("ObsSW.Observation.Beam[1].subbandList", 	"[4,3,102]");
		parSet1.add("ObsSW.Observation.Beam[1].beamletList", 	"[15,16,18]");
		Observation  obs2(&parSet1);
		cout << obs2 << endl;

		// test conflicts in clock
		ParameterSet conflictPS1("tObservation.in_conflict1");
		Observation  conflictObs1(&conflictPS1);
		ASSERTSTR(obs2.conflicts(conflictObs1), "File 1 should have had a clock conflict");
	
		// test conflicts in receivers
		ParameterSet conflictPS2("tObservation.in_conflict2");
		Observation  conflictObs2(&conflictPS2);
		ASSERTSTR(obs2.conflicts(conflictObs2), "File 2 should have had a receiver conflict");
	
		// test conflicts in beamlets
		ParameterSet conflictPS3("tObservation.in_conflict3");
		Observation  conflictObs3(&conflictPS3);
		ASSERTSTR(obs2.conflicts(conflictObs3), "File 3 should have had a beamlet conflict");
	
		// test conflicts in nrSlotsPerFrame
		ParameterSet conflictPS4("tObservation.in_conflict4");
		Observation  conflictObs4(&conflictPS4);
		ASSERTSTR(obs2.conflicts(conflictObs4), "File 4 should have had a nrSlotInFrame conflict");
	
		// everything conflict except the time
		ParameterSet conflictPS5("tObservation.in_conflict5");
		Observation  conflictObs5(&conflictPS5);
		ASSERTSTR(!obs2.conflicts(conflictObs5), "File 5 should NOT have had a conflict");
		LOG_INFO("No conflict found in file 5 which is oke.");

		// test RCUbitset based on receiverList
		bitset<MAX_RCUS>	expectedRCUs;
		expectedRCUs.reset();
		for (int r = 0; r < 12; r++) {
			expectedRCUs.set(r);
		}
		LOG_INFO_STR("getRCUbitset(48,48,12,false) = " << obs1.getRCUbitset(48,48,12,false));

		// basic test on RCU bitsets
		parSet1.replace("ObsSW.Observation.antennaSet", "LBA_OUTER");
		Observation		obs3(&parSet1);
		LOG_INFO_STR(obs3.antennaSet);
		LOG_INFO_STR("getRCUbitset(96,48,12,true)  = " << obs3.getRCUbitset(96,48,12,true));	// Core
		LOG_INFO_STR("getRCUbitset(96,48,12,false) = " << obs3.getRCUbitset(96,48,12,false));	// Remote
		LOG_INFO_STR("getRCUbitset(96,48,24,false) = " << obs3.getRCUbitset(96,48,24,false));	// Europe
		LOG_INFO_STR("getRCUbitset(96,96,24,false) = " << obs3.getRCUbitset(96,96,24,false));	// Europe
		
		// basic test on RCU bitsets
		obs3.antennaSet = "HBA_BOTH";
		LOG_INFO_STR(obs3.antennaSet);
		LOG_INFO_STR("getRCUbitset(96,48,12,true)  = " << obs3.getRCUbitset(96,48,12,true));	// Core
		LOG_INFO_STR("getRCUbitset(96,48,12,false) = " << obs3.getRCUbitset(96,48,12,false));	// Remote
		LOG_INFO_STR("getRCUbitset(96,48,24,false) = " << obs3.getRCUbitset(96,48,24,false));	// Europe
		LOG_INFO_STR("getRCUbitset(96,96,24,false) = " << obs3.getRCUbitset(96,96,24,false));	// Europe
		
		// tricky test on RCU bitsets
		obs3.antennaSet = "HBA_ONE";
		LOG_INFO_STR(obs3.antennaSet);
		LOG_INFO_STR("getRCUbitset(96,48,12,true)  = " << obs3.getRCUbitset(96,48,12,true));	// Core
		LOG_INFO_STR("getRCUbitset(96,48,12,false) = " << obs3.getRCUbitset(96,48,12,false));	// Remote
		LOG_INFO_STR("getRCUbitset(96,48,24,false) = " << obs3.getRCUbitset(96,48,24,false));	// Europe
		LOG_INFO_STR("getRCUbitset(96,96,24,false) = " << obs3.getRCUbitset(96,96,24,false));	// Europe
		
	
	}
	catch (Exception& e) {
		cout << "Exception: " << e.what() << endl;
		return 1;
	}
	return 0;
}
