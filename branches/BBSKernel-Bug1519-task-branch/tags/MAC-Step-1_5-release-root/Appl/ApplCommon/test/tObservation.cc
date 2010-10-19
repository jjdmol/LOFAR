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
#include <APS/ParameterSet.h>
#include <ApplCommon/Observation.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	try {
		string	ps1;
		ps1 += "_treeID                    = 25\n";
		ps1 += "Observation.name           = observation1\n";
		ps1 += "Observation.startTime      = 2007-01-15 12:20:00\n";
		ps1 += "Observation.stopTime       = 2007-01-15 13:45:59\n";
		ps1 += "Observation.bandFilter     = LBL_10_90\n";
		ps1 += "Observation.antennaArray   = CS1_LBA\n";
		ps1 += "Observation.receiverList   = [0..21,24,26]\n";
		ps1 += "Observation.MSNameMask     = /data/L${YEAR}_${MSNUMBER}/SB${SUBBAND}.MS\n";
		ps1 += "Observation.sampleClock    = 160\n";
		ps1 += "Observation.nrBeams        = 1\n";
		ps1 += "Observation.Beam[1].angle1 = 1.57079632679\n";
		// do not specify angle2!
		ps1 += "Observation.Beam[1].directionType= J2000\n";
		ps1 += "Observation.Beam[1].subbandList  = [5,6,100..103]\n";
		ps1 += "Observation.Beam[1].beamletList  = [0..2,20,21,215]\n";
		ps1 += "Observation.VirtualInstrument.stationList    = [CS002,CS005..CS009,CS015]\n";
		ps1 += "Observation.VirtualInstrument.BGLNodeList    = [bgl001..bgl050,bgl100]\n";
		ps1 += "Observation.VirtualInstrument.storageNodeList= [stor001,stor003]\n";

		ParameterSet	parSet1;
		parSet1.adoptBuffer(ps1);

		cout << "ParameterSet 1:" << endl;
		cout << parSet1;

		Observation	obs1(&parSet1);
		cout << obs1 << endl;

		// add an extra beam
		ps1 += "Observation.nrBeams      = 2\n";
		ps1 += "Observation.Beam[2].angle1  = 0.23456789\n";
		ps1 += "Observation.Beam[2].angle2  = 0.123456789\n";
		ps1 += "Observation.Beam[2].directionType= AZEL\n";
		ps1 += "Observation.Beam[2].subbandList  = [4,3,102]\n";
		ps1 += "Observation.Beam[2].beamletList  = [15,16,18]\n";

		ParameterSet	parSet2;
		parSet2.adoptBuffer(ps1);

		Observation	obs2(&parSet2);
		cout << obs2 << endl;
	}
	catch (Exception&	e) {
		cout << "Exception: " << e.what() << endl;
	}

	return (0);
}

