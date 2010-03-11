//# StationInfo.cc: Several 'deployment' related functions
//#
//# Copyright (C) 2006
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

#if defined HAVE_BOOST_REGEX

#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/StationInfo.h>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
using namespace boost;

namespace LOFAR {

static	char*	stationTypeTable[] = { "CS", "RS", "ES" };
static	char*	ringTypeTable[]    = { "Core", "Remote", "Europe" };

//
// stationTypeValue()
//
// Returns the stationType (0..2) of the current machine.
// The the returned value is an index in the stationTypeTable or ringTypeTable.
//
int	stationTypeValue()
{
	string	stsType(toUpper(myHostname(false).substr(0,2)));	// RS, CS, xx
	if (stsType == "CS") {
		return (0);
	}
	if (stsType == "RS") {
		return (1);
	}

	return (2);
}


//
// stationTypeStr()
//
// Returns the unified stationtype (CS | RS | ES).
//
string	stationTypeStr()
{
	return (stationTypeTable[stationTypeValue()]);
}


//
// stationRingName()
//
// Returns the name of the ring the station belongs to.
//
string	stationRingName()
{
	return (ringTypeTable[stationTypeValue()]);
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

	return (toUpper(hostname));
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
	const char*		structure_match = "(([A-Z]{2,3}[0-9]{3}[A-Z]?):LOFAR_(PIC|PermSW)_)|"		// 1,2,3
									  "(([A-Z]{2,3}[0-9]{3}[A-Z]?):LOFAR_(PIC|PermSW)\\.)";	// 4,5,6
	const char*		location_match  = "(RCU[0-9]{3})|"									// 1
									  "_(CS[0-9]{3}[A-Z]?)_|" 							// 2 CS999
									  "_(RS[0-9]{3}[A-Z]?)_|"							// 3 RS999
									  "_([ABD-QS-Z][A-Z][0-9]{3}[A-Z]?)_|"				// 4 XX999
									  "_([A-Z]{3}[0-9]{3}[A-Z]?)_";						// 5 XXX999
	const char*		separator_match = "(_)|(\\.)";
	const char*		boundary_match  = "(^([^_]+)_)";

	const char*		structure_repl  = "(?1LOFAR_$3_$2_)"	// LOFAR_PIC_RS002
									  "(?4LOFAR_$6.)";		// LOFAR_PIC
	const char*		location_repl	= "(?1$&)"				// ignore RCU999
									  "(?2_Core$&)"			 
									  "(?3_Remote$&)"
									  "(?4_Europe$&)"
									  "(?5_Control$&)";
	const char*		separator_repl  = "(?1.)(?2_)";			// swap separators
	const char*		boundary_repl   = "$2.";				// reverse separator on object-field edge

	boost::regex	strexp, locexp, sepexp, bndexp;
	locexp.assign(location_match);
	strexp.assign(structure_match);
	sepexp.assign(separator_match);
	bndexp.assign(boundary_match);

	return (boost::regex_replace(
				boost::regex_replace(
					boost::regex_replace(
						boost::regex_replace(
						PVSSname, strexp, structure_repl, boost::match_default | boost::format_all), 
					locexp, location_repl, boost::match_default | boost::format_all),
				sepexp, separator_repl, boost::match_default | boost::format_all),
			bndexp, boundary_repl, boost::match_default));
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
	const char*		structure_match = "(LOFAR\\.(PIC|PermSW)\\.(Core|Remote|Europe|Control)\\.([A-Z]{2}[0-9]{3})\\.)";		// 1,2,3
	const char*		separator_match = "(_)|(\\.)";
	const char*		boundary_match  = "(\\.([^\\.]+))$";

	const char*		structure_repl  = "(?1$4\\:LOFAR\\.$2\\.)";	// RS002:LOFAR_PIC
	const char*		separator_repl  = "(?1.)(?2_)";				// swap separators
	const char*		boundary_repl   = "_$2";					// reverse separator on object-field edge

	boost::regex	strexp, sepexp, bndexp;
	strexp.assign(structure_match);
	sepexp.assign(separator_match);
	bndexp.assign(boundary_match);

	return (boost::regex_replace(
				boost::regex_replace(
					boost::regex_replace(
					SASname, strexp, structure_repl, boost::match_default | boost::format_all),
				bndexp, boundary_repl, boost::match_default),
			sepexp, separator_repl, boost::match_default | boost::format_all));
	}

} // namespace LOFAR

#endif
