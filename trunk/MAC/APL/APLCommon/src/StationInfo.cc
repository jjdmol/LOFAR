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
#include <Common/SystemUtil.h>
#include <APL/APLCommon/StationInfo.h>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
using namespace boost;

namespace LOFAR {
  namespace Deployment {

static	char*	stationTypeTable[] = { "CS", "RS", "ES" };

//
// stationRingNr()
//
// Syntax of hostname: <stationType><arm><ring>
//	with: stationType = CS | RS | <countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//
uint16	stationRingNr()
{
	if (!isdigit(myHostname(false).substr(2,1)[0])) {
		return(0);
	}
	return (lexical_cast<uint16>(myHostname(false).substr(3,2)));
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
	if (!isdigit(myHostname(false).substr(2,1)[0])) {
		return(0);
	}
	return (lexical_cast<uint16>(myHostname(false).substr(2,1)));
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
	string	stsType(toUpper(myHostname(false).substr(0,2)));	// RS, CS, xx
	if (stsType == "CS") {
		return (0);
	}
	if (stsType == "RS") {
		return (1);
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
		return ("ES");
	}

	return (stationTypeTable[stsType]);
}


//
// PVSSDatabaseName
//
string	PVSSDatabaseName(const string&	someName)
{
	string hostname(someName);
	if (hostname.empty()) {
		hostname = myHostname(false);
	}

	// hostname is like <stationtype><arm><ring><CUtype>
	// where CUtype = C or W
	// strip off CUtype is any to get the PVSS database name
	rtrim(hostname, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

	return (hostname);
}

//
// realHostname
//
// In SAS the stationnames are used as hostnames so the last 'C' or 'T' is missing.
// This routine tries to find out if it has to add the C or T to the given hostname.
//
string	realHostname(const string&	someName)
{
	// if given name ends in a char assume the name is correct.
	char	someLastChar(*(--(someName.end())));
	if (someLastChar == 'C' || someLastChar == 'T') {
		return (someName);
	}

	// leaves us with ABC999 and XY999 formats
	// when 3rd character is not a digit the name refers to a CEP machine
	if (!isdigit(someName[2])) {
		return (someName);
	}

	// given name must be a stationname. When myhostname ends in a T
	// add the T else assume production and add the C
	string	hostname(toUpper(myHostname(false)));
	char	myLastChar(*(--(hostname.end())));
	if (myLastChar == 'T') {
		return (string(someName+'T'));
	}
	return (someName+'C');
}

//
// PVSS2SASname(PVSSname)
//
// Converts a name of a SAS datapoint to the corresponding PVSS DPname
//
// SAS :  LOFAR.PIC.<RING>.<SYSTEM>.xxx
// PVSS:  <SYSTEM>:LOFAR_PIC_xxx
//        ^       ^      ^
//        |       |      +-- locationPos + locationLen
//        |       +-- colon
//        +-- systemLen
//
// NOTE: instead of PIC the DPname may contain PermSW or ObsSW_Observation<n>
//
string PVSS2SASname(const string&	PVSSname)
{
//	This is typically something for a regular expression but I can't get it linked.
//	const char*		expression_text = "s/^([^:]*):LOFAR_PIC_";
//	const char*		format_text     = "(?1PIC.Remote.$&.)";
//	boost::regex	bexp;
//	bexp.assign(expression_text);
//	return(boost::regex_replace(PVSSname, bexp, format_text));

	size_t	colon(PVSSname.find(':',0));	// = systemLen
	size_t	locationPos(PVSSname.find("_",colon)+1);
	size_t	locationLen(PVSSname.find("_",locationPos)-locationPos);
	if (locationLen > 1024) {
		locationLen = PVSSname.find(".",locationPos)-locationPos;
	}
	string	location(PVSSname.substr(locationPos, locationLen));
	if ((location != "PIC") && (location != "PermSW") &&(location != "ObsSW")) {
		string	SASname(PVSSname);
		replace(SASname.begin(), SASname.end(), '_', '.');
		return (SASname);
	}

	string	stnCode = PVSSname.substr(0,2);
	string	ring;
	if (stnCode == "CS")
		ring = "Core";
	else if (stnCode == "RS")
		ring = "Remote";
	else if (colon == 5)
		ring = "Europe";
	else
		ring = "CEP";

	string	SASname;
	if (location == "ObsSW") {
		// skip 'Observation<n> part
		// <SYSTEM>:LOFAR_ObsSW_Observation<n>_xxx
		//                                     ^
		//                                     +-- restPos
		size_t	restPos(PVSSname.find("_", locationPos+locationLen+1)+1);
		if (restPos != string::npos) {
			//                        LOFAR.ObsSW.<RING>.<SYSTEM>.xxx
			SASname = formatString("LOFAR.%s.%s.%s.%s", location.c_str(), ring.c_str(), PVSSname.substr(0,colon).c_str(),
								PVSSname.substr(restPos).c_str());
		}
		else {
			//                        LOFAR.ObsSW.Observation.xxx
			restPos = PVSSname.find(".", locationPos+locationLen+1)+1;
			SASname = formatString("LOFAR.ObsSW.Observation.%s", PVSSname.substr(restPos).c_str());
		}
	}
	else {
		//                        LOFAR.PIC.<RING>.<SYSTEM>.xxx
		SASname = formatString("LOFAR.%s.%s.%s.%s", location.c_str(), ring.c_str(), PVSSname.substr(0,colon).c_str(), 
							PVSSname.substr(locationPos+locationLen+1).c_str());
	}
	replace(SASname.begin(), SASname.end(), '_', '.');
	return (SASname);
}

//
// SAS2PVSSname(SASname)
//
// Converts a name of a SAS datapoint to the corresponding PVSS DPname
//
// PVSS:  <SYSTEM>:LOFAR_PIC_xxx
// SAS :  LOFAR.PIC.<RING>.<SYSTEM>.xxx
//              ^   ^      ^
//              |   |      +-- systemPos + systemLen
//              |   +-- ringPos
//              +-- locationPos + locationLen
//
// NOTE: instead of PIC the DPname may contain PermSW
//
string SAS2PVSSname(const string&	SASname)
{
	string	PVSSname(SASname);									// prepare answer
	replace(PVSSname.begin(), PVSSname.end(), '.', '_');		// replace all . with _
	PVSSname.replace(PVSSname.find_last_of('_'), 1, 1, '.');	// except for the last

	// LOFAR_PIC_<ring>_<system>_xxx_yyy.zzz
	size_t	locationPos(PVSSname.find("PIC_",0));				// PIC might need adjustments
	int		locationLen = 4;
	if (locationPos == string::npos) {
		locationLen = 7;
		locationPos = PVSSname.find("PermSW_",0);
		if (locationPos == string::npos) {
			return (PVSSname);									// no PIC or PermSW DP.
		}
	}
	size_t	ringPos(locationPos + locationLen);

	// Check ringname
	string	ring = PVSSname.substr(ringPos,4);
	if (ring != "Core" && ring != "Remo" && ring != "Euro" && ring != "CEP_") {
		return (PVSSname);
	}

	size_t	systemPos(PVSSname.find("_", ringPos)+1);
	int		systemLen(PVSSname.find("_", systemPos+1) - systemPos);
	return (formatString("%s:%s%s", PVSSname.substr(systemPos, systemLen).c_str(),
							PVSSname.substr(0, ringPos).c_str(),
							PVSSname.substr(systemPos+systemLen+1).c_str()));
}




  } // namespace Deployment
} // namespace LOFAR
