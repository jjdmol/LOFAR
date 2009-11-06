//#  tDeployment.cc: Regressiontest for Deployment.cc
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Deployment.h>

using namespace std;
using namespace LOFAR;


int main (int, char *argv[]) {

	// Read in the log-environment configuration
	INIT_LOGGER(argv[0]);

	// Show operator were are on the air
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_INFO_STR("Myhostname                         = " << myHostname(false));
	LOG_INFO_STR("My full hostname                   = " << myHostname(true));
	LOG_INFO_STR("stationRingNr                      = " << stationRingNr());
	LOG_INFO_STR("stationArmNr                       = " << stationArmNr());
	LOG_INFO_STR("stationTypeValue                   = " << stationTypeValue());
	LOG_INFO_STR("stationTypeStr                     = " << stationTypeStr());
	LOG_INFO_STR("LOFAR_@ring@_@arm@_@station@_test  = " << 
				createPropertySetName ("LOFAR_@ring@_@arm@_@station@_test"));
	LOG_INFO_STR("LOFAR_@instance@_test [instance=0] = " << 
				createPropertySetName ("LOFAR_@instance@_test", 0));
	LOG_INFO_STR("LOFAR_@instance@_test [instance=25]= " << 
				createPropertySetName ("LOFAR_@instance@_test", 25));

	LOG_INFO("Normal termination of program");
	return (0);
}

