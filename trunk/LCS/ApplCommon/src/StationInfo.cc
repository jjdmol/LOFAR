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

#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <Common/ParameterSet.h>	// indexValue
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/StationInfo.h>

#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>

#if defined HAVE_BOOST_REGEX
# include <boost/regex.hpp>
#endif

using namespace boost;

namespace LOFAR {

static	const char*	stationTypeTable[] = { "CS", "RS", "ES" };
static	const char*	ringTypeTable[]    = { "Core", "Remote", "Europe" };

//
// stationTypeValue()
//
// Returns the stationType (0..2) of the current machine.
// The returned value is an index in the stationTypeTable or ringTypeTable.
//
int	stationTypeValue ()
{
        return stationTypeValue (myHostname(false));
}

// Convert a station or host name to station type.
int	stationTypeValue (const string& name)
{
	string	stsType(toUpper(name.substr(0,2)));	// RS, CS, xx
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
// getObservationNr(controllerName)
//
// Get the ObservationNr from the controllername.
uint32	getObservationNr (const string&	controllerName)
{
	return (indexValue(controllerName, "{}"));
}

//
// getInstanceNr(controllername)
//
// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	controllerName)
{
	string		cntlrName (controllerName);		// destroyable copy
	rtrim(cntlrName, "{}0123456789");
	return (indexValue(cntlrName, "[]"));
}

//
// createPropertySetName(propSetMask)
//
//  A PropSetMask may contain the markers:
//	@ring@
//	@station@
//  @instance@
//	@observation@
//	@cabinet@
//	@subrack@
//	@RSPboard@
//	@TBboard@
//	@rcu@
//      @bgp@
//      @midplane@
//      @ionode@
//      @osrack@
//      @ossubcluster@
//      @storagenode@
//      @offlinenode@
//  @inputbuffer@
//  @adder@
//  @storage@
//  @cobaltnode@
//  @cobaltnic@
//  @oscobaltnode@
//  @pscobaltnode@
//  @cobaltgpuproc@
//
string	createPropertySetName(const string&		propSetMask,
							  const string&		controllerName,
							  const string&		realDPname)
{
	string	psName(propSetMask);		// editable copy
	string::size_type	pos;
	// when name contains @ring@_@station@ cut out this marker and prepend hostname
	// stationname+:  -> LOFAR_ObsSW_@ring@_@station@_CalCtrl_xxx --> CS010:LOFAR_ObsSW_CalCtrl_xxx
	if ((pos = psName.find("@ring@_@station@_")) != string::npos) {
		psName.erase(pos, 17);
		psName = PVSSDatabaseName(myHostname(false)) + ":" + psName;
	}

	if ((pos = psName.find("@ring@")) != string::npos) {
		psName.replace(pos, 6, stationRingName());
	}

	if ((pos = psName.find("@station@")) != string::npos) {
		psName.replace(pos, 9, PVSSDatabaseName(myHostname(false)));
	}

	if ((pos = psName.find("@instance@")) != string::npos) {
		uint16	instanceNr = getInstanceNr(controllerName);
		if (instanceNr) {
			psName.replace(pos, 10, lexical_cast<string>(instanceNr));
		}
		else {
			psName.replace(pos, 10, "");	
		}
	}

	if ((pos = psName.find("LOFAR_ObsSW_@observation@")) != string::npos) {
		psName.replace(pos, 25, realDPname);
	}

	if ((pos = psName.find("@cabinet@")) != string::npos) {
		psName.replace(pos, 9, string("Cabinet%d"));
	}
	if ((pos = psName.find("@subrack@")) != string::npos) {
		psName.replace(pos, 9, string("Subrack%d"));
	}
	if ((pos = psName.find("@RSPBoard@")) != string::npos) {
		psName.replace(pos, 10, string("RSPBoard%d"));
	}
	if ((pos = psName.find("@TBBoard@")) != string::npos) {
		psName.replace(pos, 9, string("TBBoard%d"));
	}
	if ((pos = psName.find("@rcu@")) != string::npos) {
		psName.replace(pos, 5, string("RCU%d"));
	}
	if ((pos = psName.find("@bgp@")) != string::npos) {
		psName.replace(pos, 5, string("BGP%d"));
	}
	if ((pos = psName.find("@midplane@")) != string::npos) {
		psName.replace(pos, 10, string("Midplane%d"));
	}
	if ((pos = psName.find("@ionode@")) != string::npos) {
		psName.replace(pos, 8, string("IONode%02d"));
	}
	if ((pos = psName.find("@osionode@")) != string::npos) {
		psName.replace(pos, 10, string("OSIONode%02d"));
	}
	if ((pos = psName.find("@locusnode@")) != string::npos) {
		psName.replace(pos, 11, string("LocusNode%03d"));
	}
	if ((pos = psName.find("@osrack@")) != string::npos) {
		psName.replace(pos, 8, string("OSRack%d"));
	}
	if ((pos = psName.find("@ossubcluster@")) != string::npos) {
		psName.replace(pos, 14, string("OSSubcluster%d"));
	}
	if ((pos = psName.find("@storagenode@")) != string::npos) {
		psName.replace(pos, 13, string("StorageNode%d"));
	}
	if ((pos = psName.find("@offlinenode@")) != string::npos) {
		psName.replace(pos, 13, string("OfflineNode%d"));
	}
	if ((pos = psName.find("@inputbuffer@")) != string::npos) {
		psName.replace(pos, 13, string("InputBuffer%d"));
	}
	if ((pos = psName.find("@adder@")) != string::npos) {
		psName.replace(pos, 7, string("Adder%d"));
	}
	if ((pos = psName.find("@storage@")) != string::npos) {
		psName.replace(pos, 9, string("Storage%d"));
	}
	if ((pos = psName.find("@cobaltnode@")) != string::npos) {
		psName.replace(pos, 12, string("CBT%03d"));
	}
	if ((pos = psName.find("@cobaltnic@")) != string::npos) {
		psName.replace(pos, 11, string("CobaltNIC%02d"));
	}
	if ((pos = psName.find("@oscobaltnode@")) != string::npos) {
		psName.replace(pos, 14, string("OSCBT%03d"));
	}
	if ((pos = psName.find("@pscobaltnode@")) != string::npos) {
		psName.replace(pos, 14, string("PSCBT%03d"));
	}
	if ((pos = psName.find("@cobaltgpuproc@")) != string::npos) {
		psName.replace(pos, 15, string("CobaltGPUProc%02d"));
	}
		
	return (psName);
}

#if defined HAVE_BOOST_REGEX

//
// PVSS2SASname(PVSSname)
//
// Converts a name of a SAS datapoint to the corresponding PVSS DPname
//
// SAS :  LOFAR.PIC.<RING>.<SYSTEM>.xxx
// PVSS:  <SYSTEM>:LOFAR_PIC_xxx
//
// NOTE: instead of PIC the DPname may contain PermSW or ObsSW_Observation<n>
//
string PVSS2SASname(const string&	PVSSname)
{
	const char*		structure_match = "(([A-Z]{2,3}[0-9]{3}[A-Z]?):LOFAR_(PIC|PermSW)_)|"		// 1,2,3
									  "(([A-Z]{2,3}[0-9]{3}[A-Z]?):LOFAR_(PIC|PermSW)\\.)";		// 4,5,6
	const char*		location_match  = "(RCU[0-9]{3})|"									// 1
									  "(LBA[0-9]{3})|"									// 2 LBA999
									  "(HBA[0-9]{2})|"									// 3 HBA99
									  "_(CS[0-9]{3}[A-Z]?)_|" 							// 4 CS999
									  "_(RS[0-9]{3}[A-Z]?)_|"							// 5 RS999
									  "_([ABD-QS-Z][A-Z][0-9]{3}[A-Z]?)_|"				// 6 XX999
									  "_([A-Z]{3}[0-9]{3}[A-Z]?)_";						// 7 XXX999
	const char*		separator_match = "(_)|(\\.)";
	const char*		boundary_match  = "(^([^_]+)_)";

	const char*		structure_repl  = "(?1LOFAR_$3_$2_)"	// LOFAR_PIC_RS002
									  "(?4LOFAR_$6.)";		// LOFAR_PIC
	const char*		location_repl	= "(?1$&)"				// ignore RCU999
									  "(?2$&)"
									  "(?3$&)"
									  "(?4_Core$&)"			 
									  "(?5_Remote$&)"
									  "(?6_Europe$&)"
									  "(?7_Control$&)";
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

#endif /* HAVE_BOOST_REGEX */

} // namespace LOFAR

