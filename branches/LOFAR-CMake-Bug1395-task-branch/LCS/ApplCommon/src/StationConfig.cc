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
#include <ApplCommon/StationConfig.h>

namespace LOFAR {

//
// Constructor
//
StationConfig::StationConfig()
{
	// Try to find the configurationfile
	ConfigLocator   CL;
	string          fileName(CL.locate("RemoteStation.conf"));
	ASSERTSTR(!fileName.empty(), "Cannot find the station configurationfile 'RemoteStation.conf'");

	ParameterSet    StationInfo(fileName);
	stationID	 = StationInfo.getInt ("RS.STATION_ID");
	nrRSPs		 = StationInfo.getInt ("RS.N_RSPBOARDS");
	nrTBBs		 = StationInfo.getInt ("RS.N_TBBOARDS");
	nrLBAs		 = StationInfo.getInt ("RS.N_LBAS");
	nrHBAs		 = StationInfo.getInt ("RS.N_HBAS");
	hasSplitters = StationInfo.getBool("RS.HBA_SPLIT");
	hasWideLBAs	 = StationInfo.getBool("RS.WIDE_LBAS");

	LOG_DEBUG(formatString("Stations has %d LBA and %d HBA antennas and %ssplitters",
											nrLBAs, nrHBAs, (hasSplitters ? "" : "NO ")));
}

StationConfig::~StationConfig()
{ }

} // namespace LOFAR
