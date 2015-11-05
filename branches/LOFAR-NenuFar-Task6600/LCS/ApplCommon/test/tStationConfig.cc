//# tStationConfig.cc
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
//# $Id: tStationConfig.cc 22376 2012-10-17 10:00:37Z mol $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/StationConfig.h>

using namespace LOFAR;

void show (const StationConfig&	SC)
{
	cout << "StationID    = " << SC.stationID << endl;
	cout << "nr RSPboards = " << SC.nrRSPs << endl;
	cout << "nr TBboards  = " << SC.nrTBBs << endl;
	cout << "HBA split    = " << SC.hasSplitters << endl;
	cout << "wide LBAs    = " << SC.hasWideLBAs << endl;
	cout << "Aartfaac     = " << SC.hasAartfaac << endl;
	cout << "nr antenna types = " << SC.nrTypes << endl;
	cout << "antenna types= " << SC.antTypes << endl;
	for (int i = 0; i < SC.nrTypes; i++) {
		cout << "field " << SC.antTypes[i] << ": " << SC.antCounts[i] << endl;
		ASSERTSTR(SC.antCounts[i] == SC.nrAntennas(SC.antTypes[i]), "Illegal count returned for nrAntennas(" << SC.antTypes[i] << ")");
	}
}

int main (int/*argc*/, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	StationConfig	oldConfig("tStationConfig.in_oldFormat");
	show(oldConfig);
	
	StationConfig	newConfig("tStationConfig.in_newFormat");
	show(newConfig);
	
	try {
		StationConfig	wrongConfig("tStationConfig.in_wrongFormat");
	} catch (LOFAR::APSException&	ex) {
	}
	
	return (0);
}
