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
#include <APL/APLCommon/Observation.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;
using namespace LOFAR::ACC::APS;

int main (int	argc, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	string	ps1;
	ps1 += "Observation.name         = observation1\n";
	ps1 += "Observation.startTime    = 2007-01-15 12:20:00\n";
	ps1 += "Observation.stopTime     = 2007-01-15 13:45:59\n";
	ps1 += "Observation.nyquistZone  = 1\n";
	ps1 += "Observation.subbandList  = [5,6,100..103]\n";
	ps1 += "Observation.beamletList  = [0..5]\n";
	ps1 += "Observation.bandFilter   = LBL_10_90\n";
	ps1 += "Observation.antennaArray = CS1_LBA\n";
	ps1 += "Observation.receiverList = [0..21,24,26]\n";
	ps1 += "Observation.sampleClock  = 160\n";

	ParameterSet	parSet1;
	parSet1.adoptBuffer(ps1);

	cout << "ParameterSet 1:" << endl;
	cout << parSet1;

	Observation	obs1(&parSet1);
	cout << "Observation 1:" << endl;
    cout << "treeid       : " << obs1.treeID << endl;
    cout << "nyquistZone  : " << obs1.nyquistZone << endl;
	cout << "size of beamletList : " << obs1.beamlets.size() << endl;
	cout << "size of subbandList : " << obs1.subbands.size() << endl;
    cout << "subbandList  : [";
	for (vector<int16>::iterator iter = obs1.subbands.begin(); iter != obs1.subbands.end(); ++iter) {
		cout << *iter << ",";
	}
	cout << "]" << endl;
    cout << "sampleClock  : " << obs1.sampleClock << endl;
    cout << "filter       : " << obs1.filter << endl;
    cout << "antennaArray : " << obs1.antennaArray << endl;
    cout << "receiverList : " << obs1.RCUset << endl;

	return (0);
}

