//# tObservation.cc
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/Observation.h>

using namespace LOFAR;

int main (int argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	try {
		if (argc == 2) {
			ParameterSet	userPS(argv[1]);
			Observation		someObs(&userPS, true);
			cout << someObs;
			cout << "getRCUbitset(96,48,'') = " << someObs.getRCUbitset(96,48,"") << endl;	// Europe
			cout << "getRCUbitset(96,96,'') = " << someObs.getRCUbitset(96,96,"") << endl;	// Europe
			cout << "getRCUbitset(96,48,LBA_XXX) = " << someObs.getRCUbitset(96,48,"LBA_XXX") << endl;	// Core
			cout << "getRCUbitset(96,96,LBA_XXX) = " << someObs.getRCUbitset(96,96,"LBA_XXX") << endl;	// Core
			cout << "getRCUbitset(96,48,HBA_XXX) = " << someObs.getRCUbitset(96,48,"HBA_XXX") << endl;	// Core
			cout << "getRCUbitset(96,96,HBA_XXX) = " << someObs.getRCUbitset(96,96,"HBA_XXX") << endl;	// Core
			vector<int>	b2b = someObs.getBeamAllocation("RS005");
			cout << "BeamAlloc for RS005 : " << b2b << endl;
			return (0);
		}

		cout << ">>>" << endl;
		ParameterSet parSet2("tObservation.in_parset2");
		Observation  dualObs(&parSet2);
		cout << dualObs << endl;

		ParameterSet parSet1("tObservation.in_parset1");
		Observation  obs1(&parSet1);
		cout << obs1 << endl;
		cout << "<<<" << endl;

		// add an extra beam
		parSet1.replace("ObsSW.Observation.nrBeams", "2");
		parSet1.add("ObsSW.Observation.Beam[1].angle1", 		"0.23456789");
		parSet1.add("ObsSW.Observation.Beam[1].angle2", 		"0.123456789");
		parSet1.add("ObsSW.Observation.Beam[1].directionType",  "AZEL");
		parSet1.add("ObsSW.Observation.Beam[1].subbandList", 	"[4,3,102]");
		parSet1.add("ObsSW.Observation.Beam[1].beamletList", 	"[15,16,18]");
		Observation  obs2(&parSet1);
		cout << obs2 << endl;

                // test storage node assignment
                parSet1.add("ObsSW.OLAP.CNProc.phaseOnePsets", "[]");
                parSet1.add("ObsSW.OLAP.CNProc.phaseTwoPsets", "[]");
                parSet1.add("ObsSW.OLAP.CNProc.phaseThreePsets", "[]");
                parSet1.add("ObsSW.Observation.DataProducts.Output_Beamformed.enabled", "true");
                parSet1.add("ObsSW.Observation.DataProducts.Output_Beamformed.filenames", "[beam0.h5,beam1.h5]");
                parSet1.add("ObsSW.Observation.DataProducts.Output_Beamformed.locations", "[/,/]");
                Observation obs4(&parSet1);
		ASSERTSTR(obs4.streamsToStorage.size() == 2, "Each file should have its own stream to storage");

		cout << ">>>" << endl;
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
		cout << "<<<" << endl;
		cout << "No conflict found in file 5 which is oke." << endl;

		// basic test on RCU bitsets
		Observation		obs3(&parSet1);
		cout << "getRCUbitset(96,48,'') = " << obs3.getRCUbitset(96,48,"") << endl;	// Europe
		cout << "getRCUbitset(96,96,'') = " << obs3.getRCUbitset(96,96,"") << endl;	// Europe
		cout << "getRCUbitset(96,48,LBA_XXX) = " << obs3.getRCUbitset(96,48,"LBA_XXX") << endl;	// Core
		cout << "getRCUbitset(96,96,LBA_XXX) = " << obs3.getRCUbitset(96,96,"LBA_XXX") << endl;	// Core
		cout << "getRCUbitset(96,48,HBA_XXX) = " << obs3.getRCUbitset(96,48,"HBA_XXX") << endl;	// Core
		cout << "getRCUbitset(96,96,HBA_XXX) = " << obs3.getRCUbitset(96,96,"HBA_XXX") << endl;	// Core
		
		// test translation of antennaSetname
		obs3.antennaSet = "HBA_ZERO";
		cout << "HBA_ZERO(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "HBA_ZERO(true)  = " << obs3.getAntennaArrayName(true) << endl;
		obs3.antennaSet = "HBA_ONE";
		cout << "HBA_ONE(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "HBA_ONE(true)  = " << obs3.getAntennaArrayName(true) << endl;
		obs3.antennaSet = "HBA_DUAL";
		cout << "HBA_DUAL(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "HBA_DUAL(true)  = " << obs3.getAntennaArrayName(true) << endl;
		obs3.antennaSet = "HBA_JOINED";
		cout << "HBA_JOINED(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "HBA_JOINED(true)  = " << obs3.getAntennaArrayName(true) << endl;

		obs3.antennaSet = "LBA_INNER";
		cout << "LBA_INNER(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "LBA_INNER(true)  = " << obs3.getAntennaArrayName(true) << endl;
		obs3.antennaSet = "LBA_OUTER";
		cout << "LBA_OUTER(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "LBA_OUTER(true)  = " << obs3.getAntennaArrayName(true) << endl;
		obs3.antennaSet = "LBA_X";
		cout << "LBA_X(false) = " << obs3.getAntennaArrayName(false) << endl;
		cout << "LBA_X(true)  = " << obs3.getAntennaArrayName(true) << endl;

		// test old syntax agains new syntax
		cout << ">>>" << endl;
		ParameterSet oldParset("tObservation.in_oldParset");
		Observation  oldObs(&oldParset,true);
		cout << "OLD SYNTAX" << endl;
		cout << oldObs << endl;

		ParameterSet newParset("tObservation.in_newParset");
		Observation  newObs(&newParset,true);
		cout << "NEW SYNTAX" << endl;
		cout << newObs << endl;
		cout << "<<<" << endl;

	}
	catch (Exception& e) {
		cout << "Exception: " << e.what() << endl;
		return 1;
	}
	return 0;
}
