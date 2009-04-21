//#  StationConfig.cc: Interface to the RemoteStation.conf file.
//#
//#  Copyright (C) 2009
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
