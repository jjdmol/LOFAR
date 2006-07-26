//#  StationInfo.cc: Several 'deployment' related functions
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
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <Common/StringUtil.h>
#include <Deployment/StationInfo.h>
#include <GCF/Utils.h>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  using namespace GCF::Common;
  namespace Deployment {

static	char*	stationTypeTable[] = { "CS", "RS", "ES" };

//
// stationRingNr()
//
// Syntax of hostname: <stationType><arm><ring>
//	with: stationType = CS | RS | ES<countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//
uint16	stationRingNr()
{
	string	hostname(myHostname(false));		// RS999, CS999, ESXX999
	string	stationType(stationTypeStr());		// RS, CS, ES
	if (stationType == "CS") {
		return (lexical_cast<uint16>(hostname.substr(3,2)));
	}
	if (stationType == "RS") {
		return (lexical_cast<uint16>(hostname.substr(3,2)));
	}
	if (stationType == "ES") {
		return (lexical_cast<uint16>(hostname.substr(5,2)));
	}

	return (0);
}

//
// stationArmNr()
//
// Syntax of hostname: <stationType><arm><ring>
//	with: stationType = CS | RS | ES<countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//
uint16	stationArmNr()
{
	string	hostname(myHostname(false));		// RS999, CS999, ESXX999
	string	stationType(stationTypeStr());		// RS, CS, ES
	if (stationType == "CS") {
		return (lexical_cast<uint16>(hostname.substr(2,1)));
	}
	if (stationType == "RS") {
		return (lexical_cast<uint16>(hostname.substr(2,1)));
	}
	if (stationType == "ES") {
		return (lexical_cast<uint16>(hostname.substr(3,1)));
	}

	return (0);
}

//
// stationTypeValue()
//
// Syntax of hostname: <stationType><arm><ring>
//	with: stationType = CS | RS | ES<countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//
int16	stationTypeValue()
{
	string	stsType(toUpper(myHostname(false).substr(0,2)));	// RS, CS, ES
	if (stsType == "CS") {
		return (0);
	}
	if (stsType == "RS") {
		return (1);
	}
	if (stsType == "ES") {
		return (2);
	}

	return (-1);
}

//
// stationTypeStr()
//
// Syntax of hostname: <stationType><arm><ring>
//	with: stationType = CS | RS | ES<countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//
string	stationTypeStr()
{
	int16	stsType = stationTypeValue();
	if (stsType < 0) {
		return ("");
	}

	return (stationTypeTable[stsType]);
}

  } // namespace Deployment
} // namespace LOFAR
