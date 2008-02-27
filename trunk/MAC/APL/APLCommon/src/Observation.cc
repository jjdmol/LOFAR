//#  Observation.cc: one line description
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StreamUtil.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Observation.h>

namespace LOFAR {
	using namespace ACC::APS;
	namespace APLCommon {

Observation::Observation() :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0)
{
}

Observation::Observation(ParameterSet*		aParSet) :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0)
{
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.";
	LOG_TRACE_VAR_STR("'Observation' located at: " << prefix);

	name  = aParSet->getString(prefix+"name", "");
	obsID = aParSet->getInt32("_treeID", 0);

	if (aParSet->isDefined(prefix+"startTime")) {
		startTime = to_time_t(time_from_string(aParSet->getString(prefix+"startTime")));
	}
	if (aParSet->isDefined(prefix+"stopTime")) {
		stopTime = to_time_t(time_from_string(aParSet->getString(prefix+"stopTime")));
	}

	if (aParSet->isDefined(prefix+"VirtualInstrument.stationList")) {
		string stString("x=" + APLUtilities::expandedArrayString(
											aParSet->getString(prefix+"VirtualInstrument.stationList")));
		ParameterSet	stParset;
		stParset.adoptBuffer(stString);
		stations = stParset.getStringVector("x");
	}

	sampleClock = aParSet->getUint32(prefix+"sampleClock", 0);
	filter 		= aParSet->getString(prefix+"bandFilter", "");
	antennaArray= aParSet->getString(prefix+"antennaArray", "");
	nyquistZone = nyquistzoneFromFilter(filter);

	RCUset.reset();							// clear RCUset by default.
	if (aParSet->isDefined(prefix+"receiverList")) {
		string	rcuString("x=" + APLUtilities::expandedArrayString(
											aParSet->getString(prefix+"receiverList")));
		ParameterSet	rcuParset;
		rcuParset.adoptBuffer(rcuString);
		vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
		if (RCUnumbers.empty()) {			// No receivers in the list?
			RCUset.set();					// assume all receivers.
		}
		else {
			for (uint i = 0; i < RCUnumbers.size();i++) {
				RCUset.set(RCUnumbers[i]);	// set mentioned receivers
			}
		}
	}

	// get the beams info
	int32	nrBeams;
	if (aParSet->isDefined(prefix+"nrBeams")) {
		nrBeams = aParSet->getUint32(prefix+"nrBeams");
	}

	// allocate beamlet 2 beam mapping and reset to 0
	beamlet2beams.resize(4*54, 0);
	beamlet2subbands.resize(4*54, 0);

	// loop over al beams
	for (int32 beam(1) ; beam <= nrBeams; beam++) {
		Beam	newBeam;
		string	beamPrefix(prefix+formatString("Beam[%d].", beam));
		// get all fields
		newBeam.angle1 		  = aParSet->getDouble(beamPrefix+"angle1", 0.0);
		newBeam.angle2 		  = aParSet->getDouble(beamPrefix+"angle2", 0.0);
		newBeam.directionType = aParSet->getString(beamPrefix+"directionType", "");
//		newBeam.angleTimes 	  = aParSet->get(beamPrefix+"angleTimes", "[]");
		// subbandList
		string sbString("x=" + APLUtilities::expandedArrayString(aParSet->getString(beamPrefix+"subbandList","[]")));
		ParameterSet	sbParset;
		sbParset.adoptBuffer(sbString);
		newBeam.subbands = sbParset.getInt16Vector("x");
		// beamletList
		string blString("x=" + APLUtilities::expandedArrayString(aParSet->getString(beamPrefix+"beamletList", "[]")));
		ParameterSet	blParset;
		blParset.adoptBuffer(blString);
		newBeam.beamlets = blParset.getInt16Vector("x");
	
		// add beam to vector
		beams.push_back(newBeam);

		// finally update beamlet 2 beam mapping.
		for (int  i = newBeam.beamlets.size()-1 ; i >= 0; i--) {
			ASSERTSTR (beamlet2beams[newBeam.beamlets[i]] == 0, 
				"beamlet " << i << " of beam " << beam << " clashes with beamlet of other beam");
			beamlet2beams   [newBeam.beamlets[i]] = beam;
			beamlet2subbands[newBeam.beamlets[i]] = newBeam.subbands[i];
		}
	}
}


//
// ~Observation()
//
Observation::~Observation()
{
}

//
// getBeamName(beamidx): string
//
string Observation::getBeamName(uint32	beamIdx) const
{
	return (formatString("observation[%d]beam[%d]", obsID, beamIdx+1));
}

//
// nyquistzoneFromFilter(filtername)
//
uint32 Observation::nyquistzoneFromFilter(const string&	filterName)
{
	if (filterName == "LBL_10_80")		{ return(1); }
	if (filterName == "LBL_30_80")		{ return(1); }
	if (filterName == "LBH_10_80")		{ return(1); }
	if (filterName == "LBH_30_80")		{ return(1); }
	if (filterName == "HB_100_190") 	{ return(2); }
	if (filterName == "HB_170_230") 	{ return(3); }
	if (filterName == "HB_210_240") 	{ return(3); }

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
    os << "starttime    : " << to_simple_string(from_time_t(startTime)) << endl;
    os << "stoptime     : " << to_simple_string(from_time_t(stopTime)) << endl;
//    os << "stations     : " << stations << endl;
    os << "stations     : "; writeVector(os, stations, ",", "[", "]"); os << endl;
    os << "antennaArray : " << antennaArray << endl;
    os << "receiverList : " << RCUset << endl;
    os << "sampleClock  : " << sampleClock << endl;
    os << "filter       : " << filter << endl;
    os << "nyquistZone  : " << nyquistZone << endl;
    os << "nrBeams      : " << beams.size() << endl;
	for (size_t	i(0) ; i < beams.size(); i++) {
		os << formatString("Beam[%d].pointing   : %f, %f, %s\n", i, beams[i].angle1, beams[i].angle2, beams[i].directionType.c_str());
//		os << "Beam[" << i << "].subbandList: " << beams[i].subbands;
//		os << "Beam[" << i << "].beamletList: " << beams[i].beamlets;
		os << "Beam[" << i << "].subbandList: "; writeVector(os, beams[i].subbands, ",", "[", "]"); os << endl;
		os << "Beam[" << i << "].beamletList: "; writeVector(os, beams[i].beamlets, ",", "[", "]"); os << endl;
	}
//	os << "beamlet2beams: " << beamlet2beams;
	os << "beamlet2beams   : "; writeVector(os, beamlet2beams,    ",", "[", "]"); os << endl;
	os << "beamlet2subbands: "; writeVector(os, beamlet2subbands, ",", "[", "]"); os << endl;

	return (os);
}


  } // namespace APLCommon
} // namespace LOFAR
