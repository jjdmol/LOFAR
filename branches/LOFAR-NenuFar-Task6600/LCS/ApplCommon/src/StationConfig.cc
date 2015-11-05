//# StationConfig.cc: Interface to the RemoteStation.conf file.
//#
//# Copyright (C) 2009
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
#include <ApplCommon/StationConfig.h>

namespace LOFAR {

//---------- global instance of StationConfig ----------
static StationConfig* globalStationConfigInstance = 0;

StationConfig* globalStationConfig()
{
	if (globalStationConfigInstance == 0) {
		globalStationConfigInstance = new StationConfig();
	}
	return (globalStationConfigInstance);
}

//
// Constructor
//
StationConfig::StationConfig(const string& filename)
{
	// Try to find the configurationfile
	ConfigLocator   CL;
	string          fileName(CL.locate(filename));
	ASSERTSTR(!fileName.empty(), "Cannot find the station configurationfile '" << filename << "'");

	ParameterSet    StationInfo(fileName);
	stationID	 = StationInfo.getInt ("RS.STATION_ID");
	nrRSPs		 = StationInfo.getInt ("RS.N_RSPBOARDS");
	nrTBBs		 = StationInfo.getInt ("RS.N_TBBOARDS");
	hasSplitters = StationInfo.getBool("RS.HBA_SPLIT");
	hasAartfaac  = StationInfo.getBool("RS.AARTFAAC");
	hasWideLBAs	 = StationInfo.getBool("RS.WIDE_LBAS");
	if (StationInfo.isDefined("RS.ANT_TYPES")) {
		antTypes = StationInfo.getStringVector("RS.ANT_TYPES");
	}
	else {
		antTypes.push_back("LBA");
		antTypes.push_back("HBA");
	}
	nrTypes = antTypes.size();
	for (int i = 0; i < nrTypes; ++i) {
		antCounts.push_back(StationInfo.getInt(formatString("RS.N_%sS", antTypes[i].c_str())));
	}
	LOG_DEBUG_STR("AntennaFields: " << antTypes << ", counts: " << antCounts << 
			formatString(" and %ssplitters", (hasSplitters ? "" : "NO ")));

}

StationConfig::~StationConfig()
{ }

int StationConfig::nrAntennas(const string& antTypeName) const
{
	for (size_t i = 0; i < antTypes.size(); ++i) {
		if (antTypes[i] == antTypeName) {
			return (antCounts[i]);
		}
	}
	return (0);
}

} // namespace LOFAR
