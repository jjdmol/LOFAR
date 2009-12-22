//# Observation.cc: class for easy access to observation definitions
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/Observation.h>
#include <Common/lofar_set.h>

namespace LOFAR {

//
// Observation()
//
Observation::Observation() :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0),
	splitter(false)
{
}

//
// Observation(ParameterSet*)
//
Observation::Observation(ParameterSet*		aParSet) :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0),
	splitter(false)
{
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.";
	LOG_TRACE_VAR_STR("'Observation' located at: " << prefix);

	name  = aParSet->getString(prefix+"name", "");
	obsID = aParSet->getInt32("_treeID", 0);
	realPVSSdatapoint = aParSet->getString("_DPname","NOT_THE_REAL_DPNAME");
#if !defined HAVE_BGL
	try {
		if (aParSet->isDefined(prefix+"startTime")) {
			startTime = to_time_t(time_from_string(aParSet->getString(prefix+"startTime")));
		}
	} catch( boost::bad_lexical_cast ) {
		THROW( Exception, prefix << "startTime cannot be parsed as a valid time string. Please use YYYY-MM-DD HH:MM:SS[.hhh]." );
	}
	try {
		if (aParSet->isDefined(prefix+"stopTime")) {
			stopTime = to_time_t(time_from_string(aParSet->getString(prefix+"stopTime")));
		}
	} catch( boost::bad_lexical_cast ) {
		THROW( Exception, prefix << "stopTime cannot be parsed as a valid time string. Please use YYYY-MM-DD HH:MM:SS[.hhh]." );
	}
#endif
	if (aParSet->isDefined(prefix+"VirtualInstrument.stationList")) {
		stationList = compactedArrayString(aParSet->getString(prefix+"VirtualInstrument.stationList"));
		stations    = aParSet->getStringVector(prefix+"VirtualInstrument.stationList", true);	// true:expandable
	}

	sampleClock = aParSet->getUint32(prefix+"sampleClock",  0);
	filter 		= aParSet->getString(prefix+"bandFilter",   "");
	antennaArray= aParSet->getString(prefix+"antennaArray", "");
	MSNameMask  = aParSet->getString(prefix+"MSNameMask",   "");
	nyquistZone = nyquistzoneFromFilter(filter);

	// new way of specifying the receivers and choosing the antenna array.
	antennaSet  	 = aParSet->getString(prefix+"antennaSet", "");
	useLongBaselines = aParSet->getBool  (prefix+"longBaselines", false);

	// auto select the right antennaArray when antennaSet variable is used.
	if (!antennaSet.empty()) {
		if (antennaSet == "HBA_BOTH") {
			antennaArray = "HBA";
			splitter = true;
		}
		else if (antennaSet == "HBA_ONE") {
			antennaArray = "HBA";
		}
		else {
			antennaArray = "LBA";
		}
	}

	RCUset.reset();							// clear RCUset by default.
	if (aParSet->isDefined(prefix+"receiverList")) {
		receiverList = aParSet->getString(prefix+"receiverList");
		vector<uint32> RCUnumbers (aParSet->getUint32Vector(prefix+"receiverList", true));	// true:expandable
		for (uint i = 0; i < RCUnumbers.size();i++) {
			RCUset.set(RCUnumbers[i]);	// set mentioned receivers
		}
	}

	BGLNodeList     = compactedArrayString(aParSet->getString(prefix+"VirtualInstrument.BGLNodeList","[]"));
	storageNodeList = compactedArrayString(aParSet->getString(prefix+"VirtualInstrument.storageNodeList","[]"));
}


//
// ~Observation()
//
Observation::~Observation()
{
}

//
// conflicts (Observation& other)
//
// check if the given Observation conflicts with this one
bool	Observation::conflicts(const	Observation&	other) const
{
#if defined HAVE_BGL
	LOG_WARN("BG/P code cannot check for conflicts between observations!!!");
	return (false);
#endif

	// if observations don't overlap they don't conflict per definition.
	if ((other.stopTime <= startTime) || (other.startTime >= stopTime)) {
		return (false);
	}

	// Observation overlap, check clock
	if (other.sampleClock != sampleClock) {
		LOG_INFO_STR("Clock of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// Observation overlap, check splitters
	if (other.splitter != splitter) {
		LOG_INFO_STR("Splitters of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// if rcumode differs there may be no overlap in receivers.
	if (other.filter != filter) {
		RCUset_t	resultSet(RCUset & other.RCUset);
		if (resultSet.any()) {
			LOG_INFO_STR("Conflicting use of receivers between observation " << obsID << 
						 " and " << other.obsID);
			LOG_DEBUG_STR("receiverConflict: " << resultSet);
			return (true);
		}
	}

	return (false);	// no conflicts
}

//
// getRCUbitset(nrLBAs, nrHBAs, nrRSPs): bitset
//
// Returns a bitset containing the RCU's requested by the observation.
//
bitset<MAX_RCUS> Observation::getRCUbitset(int nrLBAs, int nrHBAs, int nrRSPs, bool	hasSplitters)
{
	#define MAX2(a,b) ((a) > (b) ? (a) : (b))

	if (antennaSet.empty() || ((nrLBAs+nrHBAs+nrRSPs) == 0)) {		// old ParameterSet or force no interpretation?
		return (RCUset);			// return old info.
	}

	// HBA's in Core stations sometimes use half of the rcus.
	bool	fullStation(MAX2(nrLBAs, nrHBAs) <= (nrRSPs * NR_ANTENNAS_PER_RSPBOARD));
	int		nrAnts   = ((antennaSet.find("LBA") == 0) ? nrLBAs : nrHBAs);
	int		firstRCU = 0;
	if (!fullStation && (antennaSet.find("LBA") == 0)) {
		nrAnts /= 2;
	}
	else if (hasSplitters && (antennaSet == "HBA_ONE")) {
		nrAnts /= 2;
	}
	else if (hasSplitters && (antennaSet == "HBA_TWO")) {
		nrAnts /= 2;
		firstRCU = nrAnts;
	}
	
	// Set up the RCUbits. Remember that we don't care here which of the three inputs is used.
	RCUset.reset();
	for (int rcu = firstRCU; rcu < 2*nrAnts; rcu++) {
			RCUset.set(rcu);
	}
	return (RCUset);
}

//
// TEMP HACK TO GET THE ANTENNAARRAYNAME
//
string Observation::getAntennaArrayName(bool hasSplitters) const
{
	string	result;
	if (antennaSet.empty()) {
		result = antennaArray;
	}
	else {
		result = antennaSet;
	}
	
	if (!hasSplitters) {		// no splitter, always use all HBA
		if (result.find("HBA") == 0) return ("HBA");
	}
	else {						// has splitter, translate SAS names to AntennaArray.conf names
		if (result == "HBA_ONE") 	return ("HBA_0");
		if (result == "HBA_TWO") 	return ("HBA_1");
		if (result == "HBA_BOTH")	return ("HBA");
	}

	return (result);
}	

//
// getBeamName(beamidx): string
//
string Observation::getBeamName(uint32	beamIdx) const
{
	return (formatString("observation[%d]beam[%d]", obsID, beamIdx+1));
}

//
//
// nyquistzoneFromFilter(filtername)
//
uint32 Observation::nyquistzoneFromFilter(const string&	filterName)
{
	// support of old names
	if (filterName == "LBL_10_80")		{ return(1); }
	if (filterName == "LBL_30_80")		{ return(1); }
	if (filterName == "LBH_10_80")		{ return(1); }
	if (filterName == "LBH_30_80")		{ return(1); }
	if (filterName == "HB_100_190") 	{ return(2); }
	if (filterName == "HB_170_230") 	{ return(3); }
	if (filterName == "HB_210_240") 	{ return(3); }

	// support of new names
	if (filterName == "LBA_30_80")		{ return(1); }
	if (filterName == "LBA_10_90")		{ return(1); }
	if (filterName == "HBA_110_190") 	{ return(2); }
	if (filterName == "HBA_170_230") 	{ return(3); }
	if (filterName == "HBA_210_250") 	{ return(3); }

	LOG_ERROR_STR ("filterselection value '" << filterName << 
											"' not recognized, using LBL_10_80");
	return (1);
}

//
// print (os)
//
ostream& Observation::print (ostream&	os) const
{
	os << endl;
	os << "Observation  : " << name << endl;
    os << "ObsID        : " << obsID << endl;
#if !defined HAVE_BGL
    os << "starttime    : " << to_simple_string(from_time_t(startTime)) << endl;
    os << "stoptime     : " << to_simple_string(from_time_t(stopTime)) << endl;
#endif
//    os << "stations     : " << stations << endl;
    os << "stations     : "; writeVector(os, stations, ",", "[", "]"); os << endl;
    os << "antennaArray : " << antennaArray << endl;
    os << "antenna set  : " << antennaSet << endl;
    os << "receiver set : " << RCUset << endl;
    os << "sampleClock  : " << sampleClock << endl;
    os << "filter       : " << filter << endl;
    os << "nyquistZone  : " << nyquistZone << endl << endl;
    os << "Meas.set     : " << MSNameMask << endl << endl;

	os << "Receivers    : " << receiverList << endl;
	os << "Stations     : " << stationList << endl;
	os << "BLG nodes    : " << BGLNodeList << endl;
	os << "Storage nodes: " << storageNodeList << endl << endl;

	return (os);
}


} // namespace LOFAR
