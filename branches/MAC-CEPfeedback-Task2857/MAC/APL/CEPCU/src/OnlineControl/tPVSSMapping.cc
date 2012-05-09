//#  tPVSSmapping.cc: test StreamToStrorage conversion to PVSS dps.
//#
//#  Copyright (C) 2006
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
//#  $Id: OnlineControl.cc 18153 2011-05-31 23:03:25Z schoenmakers $
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <signal.h>
#include <Common/StreamUtil.h>
//#include <Common/lofar_vector.h>
//#include <Common/lofar_string.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/Observation.h>

using namespace std;
using namespace LOFAR;
	
int main(int argc, char* argv[])
{
	if (argc < 2) {
		cout << "Syntax: " << argv[0] << " parameterSet" << endl;
		return (-1);
	}

	ParameterSet	thePS(argv[1]);
	Observation		theObs(&thePS, false);
	int	nrStreams = theObs.streamsToStorage.size();
	cout << "_setupBGPmapping: " << nrStreams << " streams found." << endl;
	cout << "ioNode , locusNodes , adders , writers , dataProducts, dataProductTypes" << endl;

	uint	prevPset = (nrStreams ? theObs.streamsToStorage[0].sourcePset : -1);
	vector<string>	locusVector;
	vector<int>		adderVector;
	vector<int>		writerVector;
	vector<string>	DPVector;
	vector<string>	DPtypeVector;
	for (int i = 0; i < nrStreams; i++) {
		if (theObs.streamsToStorage[i].sourcePset != prevPset) {	// other Pset? write current vector to the database.
			stringstream	os;
			writeVector(os, locusVector);
			os << ",";
			writeVector(os, adderVector);
			os << ",";
			writeVector(os, writerVector);
			os << ",";
			writeVector(os, DPVector);
			os << ",";
			writeVector(os, DPtypeVector);
			cout << prevPset << "," << os.str() << endl;
			// clear the collecting vectors
			locusVector.clear();
			adderVector.clear();
			writerVector.clear();
			DPVector.clear();
			DPtypeVector.clear();
			prevPset = theObs.streamsToStorage[i].sourcePset;
		}
		// extend vector with info
		locusVector.push_back (theObs.streamsToStorage[i].destStorageNode);
		adderVector.push_back (theObs.streamsToStorage[i].adderNr);
		writerVector.push_back(theObs.streamsToStorage[i].writerNr);
		DPVector.push_back    (theObs.streamsToStorage[i].filename);
		DPtypeVector.push_back(theObs.streamsToStorage[i].dataProduct);
	}
	return (0);
}

