//# tStationInfo.cc
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

#if defined HAVE_BOOST_REGEX

#include <Common/SystemUtil.h>
#include <ApplCommon/StationInfo.h>
#include <boost/regex.hpp>

using namespace LOFAR;

int main (int/*argc*/, char* argv[]) 
{
	INIT_LOGGER(argv[0]);
	
	LOG_INFO_STR("myHostname(short) = " << myHostname(false));
	LOG_INFO_STR("myHostname(long)  = " << myHostname(true));
	LOG_INFO_STR("ringName = " << stationRingName());
	LOG_INFO_STR("PVSSDBname = " << PVSSDatabaseName());
	LOG_INFO_STR("hostname of CS010  = " << realHostname("CS010"));
	LOG_INFO_STR("hostname of CS010C = " << realHostname("CS010C"));
	LOG_INFO_STR("hostname of CS010T = " << realHostname("CS010T"));

	LOG_INFO_STR("PVSS==>SAS(RS002:LOFAR_PIC_Cabinet0_Subrack0.status.state)        = " << 
				PVSS2SASname("RS002:LOFAR_PIC_Cabinet0_Subrack0.status.state"));
	LOG_INFO_STR("PVSS==>SAS(MCU001:LOFAR_PermSW.status.state)                      = " << 
				PVSS2SASname("MCU001:LOFAR_PermSW.status.state"));
	LOG_INFO_STR("PVSS==>SAS(RS002:LOFAR_PermSW.status.state)                       = " << 
				PVSS2SASname("RS002:LOFAR_PermSW.status.state"));
	LOG_INFO_STR("PVSS==>SAS(MCU001:LOFAR_PermSW_MACScheduler.status.state)         = " << 
				PVSS2SASname("MCU001:LOFAR_PermSW_MACScheduler.status.state"));
	LOG_INFO_STR("PVSS==>SAS(MCU001:LOFAR_ObsSW_Observation5_ObservationControl.status.state)  = " << 
				PVSS2SASname("MCU001:LOFAR_ObsSW_Observation5_ObservationControl.status.state"));
	LOG_INFO_STR("PVSS==>SAS(RS002:LOFAR_ObsSW_Observation5.antennaArray)    = " << 
				PVSS2SASname("RS002:LOFAR_ObsSW_Observation5.antennaArray"));
	LOG_INFO_STR("PVSS==>SAS(RS002:LOFAR_ObsSW_Observation5_BeamControl.status.state)  = " << 
				PVSS2SASname("RS002:LOFAR_ObsSW_Observation5_BeamControl.status.state"));
	LOG_INFO_STR("PVSS==>SAS(RS005:LOFAR_PIC_HBA05.status.state)         = " << 
				PVSS2SASname("RS005:LOFAR_PIC_HBA05.status.state"));
	LOG_INFO_STR("PVSS==>SAS(DE603:LOFAR_PIC_LBA005.status.state)         = " << 
				PVSS2SASname("DE603:LOFAR_PIC_LBA005.status.state"));
	LOG_INFO_STR("PVSS==>SAS(CS101:LOFAR_PIC_HBA01.element00.status.state)         = " << 
				PVSS2SASname("CS101:LOFAR_PIC_HBA01.element00.status.state"));
	LOG_INFO_STR("PVSS==>SAS(CS101:LOFAR_PIC_HBA02.element12.X.status.state)         = " << 
				PVSS2SASname("CS101:LOFAR_PIC_HBA02.element12.X.status.state"));

	LOG_INFO_STR("SAS==>PVSS(LOFAR.PIC.Remote.RS002.Cabinet0.Subrack0.status_state)        = " << 
				SAS2PVSSname("LOFAR.PIC.Remote.RS002.Cabinet0.Subrack0.status_state"));
	LOG_INFO_STR("SAS==>PVSS(LOFAR.PermSW.Remote.RS002.ServiceBroker.status_state)         = " << 
				SAS2PVSSname("LOFAR.PermSW.Remote.RS002.ServiceBroker.status_state"));
	LOG_INFO_STR("SAS==>PVSS(LOFAR.PermSW.Control.MCU001.MACScheduler.status_state)        = " << 
				SAS2PVSSname("LOFAR.PermSW.Control.MCU001.MACScheduler.status_state"));
	LOG_INFO_STR("SAS==>PVSS(LOFAR.ObsSW.Observation.VirtualInstrument.stationList) = " << 
				SAS2PVSSname("LOFAR.ObsSW.Observation.VirtualInstrument.stationList"));
	LOG_INFO_STR("SAS==>PVSS(LOFAR.PIC.Remote.RS005.LBA123.status_state)        = " << 
				SAS2PVSSname("LOFAR.PIC.Remote.RS005.LBA123.status_state"));
	LOG_INFO_STR("SAS==>PVSS(LOFAR.PIC.Europe.DE603.HBA23.status_state)        = " << 
				SAS2PVSSname("LOFAR.PIC.Europe.DE603.HBA23.status_state"));

	return (0);
}

#else

int main (int/*argc*/, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	return 0;
}

#endif
