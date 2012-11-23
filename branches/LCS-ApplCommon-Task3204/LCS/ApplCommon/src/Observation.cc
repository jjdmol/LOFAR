//# Observation.cc: class for easy access to observation definitions
//#
//# Copyright (C) 2006-2012
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
#include <ApplCommon/lofar_datetime.h>
#include <Common/lofar_set.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <Common/LofarBitModeInfo.h>
#include <ApplCommon/Observation.h>

#include <Common/lofar_map.h>
#include <boost/format.hpp>

using boost::format;

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
	splitterOn(false),
	itsStnHasDualHBA(false)
{ }

//
// Observation(ParameterSet*, [hasDualHBA]))
//
Observation::Observation(const ParameterSet*		aParSet,
						 bool				hasDualHBA,
						 unsigned			nrBGPIOnodes) :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0),
    bitsPerSample(0),
	splitterOn(false),
	itsStnHasDualHBA(hasDualHBA)
{
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.";
	LOG_TRACE_VAR_STR("'Observation' located at: " << prefix);

	string olapprefix = aParSet->locateModule("OLAP") + "OLAP.";
	LOG_TRACE_VAR_STR("'OLAP' located at: " << olapprefix);

	name  = aParSet->getString(prefix+"name", "");
	obsID = aParSet->getInt32("_treeID", 0);
	realPVSSdatapoint = aParSet->getString("_DPname","NOT_THE_REAL_DPNAME");

	// Start and stop times
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

	// stationlist(s)
	if (aParSet->isDefined(prefix+"VirtualInstrument.stationList")) {
		stationList = aParSet->getString(prefix+"VirtualInstrument.stationList");
		stations    = aParSet->getStringVector(prefix+"VirtualInstrument.stationList", true);	// true:expandable
		std::sort(stations.begin(), stations.end());
	}

	// miscellaneous
	sampleClock   = aParSet->getUint32(prefix+"sampleClock",  0);
	filter 		  = aParSet->getString(prefix+"bandFilter",   "");
	antennaArray  = aParSet->getString(prefix+"antennaArray", "");
	processType   = aParSet->getString(prefix+"processType", "");
	processSubtype= aParSet->getString(prefix+"processSubtype", "");
	strategy	  = aParSet->getString(prefix+"strategy", "");

    if (aParSet->isDefined(prefix+"nrBitsPerSample")) {
      bitsPerSample = aParSet->getUint32(prefix+"nrBitsPerSample", 16);
    } else {
      // backward compatibility
      LOG_WARN("Could not find Observation.nrBitsPerSample, using depricated OLAP.nrBitsPerSample");
      bitsPerSample = aParSet->getUint32(olapprefix+"nrBitsPerSample", 16);
    }

	nyquistZone = nyquistzoneFromFilter(filter);

	// new way of specifying the receivers and choosing the antenna array.
	antennaSet  	 = aParSet->getString(prefix+"antennaSet", "");
	useLongBaselines = aParSet->getBool  (prefix+"longBaselines", false);

	// auto select the right antennaArray when antennaSet variable is used.
	if (!antennaSet.empty()) {
		antennaArray = antennaSet.substr(0,3);	// LBA or HBA
	}
	splitterOn = ((antennaSet.substr(0,8) == "HBA_ZERO") || (antennaSet.substr(0,7) == "HBA_ONE") || 
				  (antennaSet.substr(0,8) == "HBA_DUAL"));
	dualMode   =  (antennaSet.substr(0,8) == "HBA_DUAL");
	bool innerMode = (antennaSet.find("_INNER") != string::npos);

	// RCU information
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

	// construct array with usable (-1) slots and unusable(999) slots. Unusable slots arise
	// when nrSlotsInFrame differs from maxBeamletsPerRSP.
	itsSlotTemplate.resize (maxBeamlets(bitsPerSample), -1);	// assume all are usable.
	nrSlotsInFrame = aParSet->getInt(prefix+"nrSlotsInFrame", maxBeamletsPerRSP(bitsPerSample));
    for (int rsp = 0; rsp < 4; rsp++) {
        for (int bl = nrSlotsInFrame; bl < maxBeamletsPerRSP(bitsPerSample); bl++) {
            itsSlotTemplate[rsp * maxBeamletsPerRSP(bitsPerSample) + bl] = 999;
        }
    }

	// determine if DataslotLists are available in this parset
	itsHasDataslots = _hasDataSlots(aParSet);
	if (itsHasDataslots) {
		itsDataslotParset = aParSet->makeSubset(prefix+"Dataslots.");		// save subset for later
	}
	else {	// init old arrays.
		itsDataslotParset = aParSet->makeSubset(prefix+"Beam", "Beam");		// save subset for later
		beamlet2beams = itsSlotTemplate;
	}
		
	//
	// NOTE: THE DATAMODEL USED IN SAS IS NOT RIGHT. IT SUPPORTS ONLY 1 POINTING PER BEAM.
	//		 THIS IS NO PROBLEM BECAUSE THAT IS WHAT WE AGREED.
	//		 WITHIN OBSERVATION HOWEVER WE USE THE RIGHT DATAMODEL THAT IS ALMOST THE SAME AS
	//		 THE BEAMSERVER USES.
	//

	// NOTE: One of the nasty problems we solve in this class is the duplication of a beam when it
	//		 is allocated on the HBA_DUAL antennaSet. For the user its one setting but for all software
	//		 it are two beams, each on a different antennaField. When HBA_DUAL is used in this observation
	//		 the corresponding beam(s) are added twice to the beam vectors, once on the HBA_ZERO antennaSet
	//		 once in the HBA_ONE antennaSet.

	// loop over all digital beams
	vector<int>		BeamBeamlets;
	int32	nrBeams = aParSet->getInt32(prefix+"nrBeams", 0);		// theoretical number
	while (nrBeams > 0 && !aParSet->isDefined(prefix+formatString("Beam[%d].angle1", nrBeams-1))) {	// check reality
		nrBeams--;
	}
	for (int32 beamIdx(0) ; beamIdx < nrBeams; beamIdx++) {
		Beam	newBeam;
		string	beamPrefix(prefix+formatString("Beam[%d].", beamIdx));
		newBeam.momID	 		 = aParSet->getInt        (beamPrefix+"momID", 0);
		newBeam.target	 		 = aParSet->getString     (beamPrefix+"target", "");
		newBeam.subbands 		 = aParSet->getInt32Vector(beamPrefix+"subbandList", vector<int32>(), true);// true:expand
		newBeam.name = getBeamName(beamIdx);
		newBeam.antennaSet = antennaSet;
		if (dualMode) {
			newBeam.antennaSet = string("HBA_ZERO") + (innerMode ? "_INNER" : "");
			if (hasDualHBA) {
				newBeam.name += "_0";
			}
		}

		// Only ONE pointing per beam.
		Pointing		newPt;
		newPt.angle1 		= aParSet->getDouble(beamPrefix+"angle1", 0.0);
		newPt.angle2 		= aParSet->getDouble(beamPrefix+"angle2", 0.0);
		newPt.directionType = aParSet->getString(beamPrefix+"directionType", "");
		newPt.duration	    = aParSet->getInt	(beamPrefix+"duration", 0);
		newPt.startTime 	= startTime;	// assume time of observation itself
		try {
			string	timeStr = aParSet->getString(beamPrefix+"startTime","");
			if (!timeStr.empty() && timeStr != "0") {
				newPt.startTime = to_time_t(time_from_string(timeStr));
			}
		} catch (boost::bad_lexical_cast) {
			LOG_ERROR_STR("Starttime of pointing of beam " << beamIdx << " not valid, using starttime of observation");
		}
		newBeam.pointings.push_back(newPt);

		// Add TiedArrayBeam information
		newBeam.nrTABs	    = aParSet->getInt   (beamPrefix+"nrTiedArrayBeams", 0);
		newBeam.nrTABrings  = aParSet->getInt   (beamPrefix+"nrTabRings", 0);
		newBeam.TABringSize = aParSet->getDouble(beamPrefix+"tabRingSize", 0.0);
		for (int32	tabIdx(0); tabIdx < newBeam.nrTABs; tabIdx++) {
			TiedArrayBeam	newTAB;
			string	tabPrefix(beamPrefix+formatString("TiedArrayBeam[%d].", tabIdx));
			newTAB.angle1			 = aParSet->getDouble(tabPrefix+"angle1", 0.0);
			newTAB.angle2			 = aParSet->getDouble(tabPrefix+"angle2", 0.0);
			newTAB.directionType     = aParSet->getString(tabPrefix+"directionType", "");
			newTAB.dispersionMeasure = aParSet->getDouble(tabPrefix+"dispersionMeasure", 0.0);
			newTAB.coherent 		 = aParSet->getBool  (tabPrefix+"coherent", false);
			newBeam.TABs.push_back(newTAB);
		}

		// Finally add the beam to the vector
		beams.push_back(newBeam);
		
		// Duplicate beam on second HBA subfield when in HBA_DUAL mode.
		if (dualMode && hasDualHBA) {
			newBeam.name	   = getBeamName(beamIdx) + "_1";
			newBeam.antennaSet = string("HBA_ONE") + (innerMode ? "_INNER" : "");
			beams.push_back(newBeam);
		}

		// finally update vector with beamnumbers
		if (_isStationName(myHostname(false))) {
			int	nrSubbands = newBeam.subbands.size();
			if (!itsHasDataslots) {		// old situation
				BeamBeamlets = aParSet->getInt32Vector(beamPrefix+"beamletList", vector<int32>(), true);	// true:expandable
				int nrBeamlets = BeamBeamlets.size();
				ASSERTSTR(nrBeamlets == nrSubbands, "Number of beamlets(" << nrBeamlets << ") != nr of subbands(" << nrSubbands << ") for Beam " << beamIdx);
				for (int  i = 0; i < nrBeamlets; ++i) {
					if (beamlet2beams[BeamBeamlets[i]] != -1) {
						stringstream	os;
						os << "beamlet2beams   : "; writeVector(os, beamlet2beams,    ",", "[", "]"); os << endl;
						LOG_ERROR_STR(os.str());
						THROW (Exception, "beamlet " << i << "(" << BeamBeamlets[i] << ") of beam " << beamIdx << " clashes with beamlet of other beam"); 
					}
					beamlet2beams[BeamBeamlets[i]] = beamIdx;
				} // for all beamlets
			}
			else { // new situation
				for (int  i = 0; i < nrSubbands; ++i) {	// Note nrBeamlets=nrSubbands
					itsBeamSlotList.push_back(beamIdx);
				}
			} // itsHasDataslots
		} // on a station
	} // for all digital beams

	// loop over al analogue beams
	int32	nrAnaBeams = aParSet->getInt32(prefix+"nrAnaBeams", 0);		// theoretical number
	while (nrAnaBeams > 0 && !aParSet->isDefined(prefix+formatString("AnaBeam[%d].angle1", nrAnaBeams-1))) {	// check reality
		nrAnaBeams--;
	}
	for (int32 beamIdx(0) ; beamIdx < nrAnaBeams; beamIdx++) {
		AnaBeam	newBeam;
		string	beamPrefix(prefix+formatString("AnaBeam[%d].", beamIdx));
		newBeam.rank	   = aParSet->getInt   (beamPrefix+"rank", 5);
		newBeam.name 	   = getAnaBeamName();
		newBeam.antennaSet = antennaSet;
		if (dualMode) {
			newBeam.antennaSet = string("HBA_ZERO") + (innerMode ? "_INNER" : "");
			if (hasDualHBA) {
				newBeam.name += "_0";
			}
		}

		// ONLY one pointing per beam.
		Pointing		newPt;
		newPt.angle1 	    = aParSet->getDouble(beamPrefix+"angle1", 0.0);
		newPt.angle2 		= aParSet->getDouble(beamPrefix+"angle2", 0.0);
		newPt.directionType = aParSet->getString(beamPrefix+"directionType", "");
		newPt.duration	    = aParSet->getInt   (beamPrefix+"duration", 0);
		newPt.startTime 	= startTime;	// assume time of observation itself
		try {
			string	timeStr = aParSet->getString(beamPrefix+"startTime","");
			if (!timeStr.empty() && timeStr != "0") {
				newPt.startTime = to_time_t(time_from_string(timeStr));
			}
		} catch (boost::bad_lexical_cast) {
			LOG_ERROR_STR("Starttime of pointing of analogue beam " << beamIdx << " not valid, using starttime of observation");
		}
		newBeam.pointings.push_back(newPt);
		
		// add beam to the analogue beam vector
		anaBeams.push_back(newBeam);

		// Duplicate beam on second HBA subfield when in HBA_DUAL mode.
		if (dualMode && hasDualHBA) {
			newBeam.name	   = getBeamName(beamIdx) + "_1";
			newBeam.antennaSet = string("HBA_ONE") + (innerMode ? "_INNER" : "");
			anaBeams.push_back(newBeam);
		}
	} // for all analogue beams

        // loop over all data products and generate all data flows
	if (!olapprefix.empty()) {		// offline Pipelines don't have OLAP in the parset.
		const char *dataProductNames[] = { "Beamformed", "Correlated" };
		unsigned dataProductPhases[]   = { 3,            2 };
        unsigned dataProductNrs[]      = { 2,            1 };
		size_t nrDataProducts = sizeof dataProductNames / sizeof dataProductNames[0];

        const unsigned nrPsets = nrBGPIOnodes;

		// by default, use all psets
		vector<unsigned> phaseTwoPsets;
		if (aParSet->isDefined(olapprefix+"CNProc.phaseTwoPsets")) {
			phaseTwoPsets = aParSet->getUint32Vector(olapprefix+"CNProc.phaseTwoPsets", true);
		}
		if (phaseTwoPsets.empty())  {
			for (unsigned p = 0; p < nrPsets; p++) {
				phaseTwoPsets.push_back(p);
			}
		}

		// by default, use all psets
		vector<unsigned> phaseThreePsets;
		if (aParSet->isDefined(olapprefix+"CNProc.phaseThreePsets")) {
			phaseThreePsets = aParSet->getUint32Vector(olapprefix+"CNProc.phaseThreePsets", true);
		}
		if (phaseThreePsets.empty())  {
			for (unsigned p = 0; p < nrPsets; p++) {
				phaseThreePsets.push_back(p);
			}
		}

		std::map<unsigned,    unsigned> filesPerIONode;
		std::map<std::string, unsigned> filesPerStorage;

		for (size_t d = 0; d < nrDataProducts; d ++) {
			bool enabled = aParSet->getBool(prefix+str(format("DataProducts.Output_%s.enabled") % dataProductNames[d]), false);

			if (!enabled)
				continue;

			// phase 2: files are ordered by beam, subband  
			// phase 3: files are ordered by beam, pencil, stokes, part

			// The .locations parset value contains the storage nodes which
			// will store each file.

			// The I/O node will allocate the files in order depth-wise.
			// That is, we determine the maximum number of files to output per
			// pset, and then proceed to fill up the I/O nodes starting from
			// the first pset. Each data product is treated individually.

			vector<string> filenames = aParSet->getStringVector(prefix+str(format("DataProducts.Output_%s.filenames") % dataProductNames[d]), true);
			vector<string> locations = aParSet->getStringVector(prefix+str(format("DataProducts.Output_%s.locations") % dataProductNames[d]), true);
			vector<unsigned> &psets = dataProductPhases[d] == 2 ? phaseTwoPsets : phaseThreePsets;

			ASSERTSTR(filenames.size() == locations.size(), "Parset provides " << filenames.size() << " filenames but only " << locations.size() << " locations.");

			unsigned numFiles = filenames.size();
			unsigned filesPerPset = (numFiles + psets.size() - 1) / psets.size();

			for (size_t i = 0; i < filenames.size(); i++) {
				StreamToStorage a;

				a.dataProduct = dataProductNames[d];
				a.dataProductNr = dataProductNrs[d];
				a.streamNr = i;
				a.filename = filenames[i];
				a.sourcePset = psets[i / filesPerPset];

				vector<string> locparts = StringUtil::split(locations[i],':');
			    ASSERTSTR(locparts.size() == 2, "A DataProduct location must be of the format host:directory (but I found " << locations[i] << ")");

				a.destStorageNode = locparts[0];
				a.destDirectory = locparts[1];

				// use a static allocation for now, starting at 0 for each pset/locus node
				a.adderNr  = filesPerIONode[a.sourcePset]++;
				a.writerNr = filesPerStorage[locparts[0]]++;

				streamsToStorage.push_back(a);
			} // for filenames
		} // for nrDataProducts
	}
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
	// if observations don't overlap they don't conflict per definition.
	if ((other.stopTime <= startTime) || (other.startTime >= stopTime)) {
		return (false);
	}

	// Observation overlap, check clock
	if (other.sampleClock != sampleClock) {
		LOG_INFO_STR("Clock of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// Observation overlap, check bit mode
	if (other.bitsPerSample != bitsPerSample) {
		LOG_INFO_STR("Bit mode of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// Observation overlap, check splitters
	if (other.splitterOn != splitterOn) {
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

	// for now also check nr of slots in frame. In the future we might allow
	// different slotsinFrame for each RSPboard but for now we treat it as a conflict.
	if (nrSlotsInFrame != other.nrSlotsInFrame) {
		LOG_INFO_STR("Conflict in nrSlotsInFrame: " << nrSlotsInFrame << "<->"  <<
					other.nrSlotsInFrame << " for resp. observation " << obsID << 
					" and " << other.obsID);
		return (true);
	}

	// check beamlets overlap
	vector<int> thisb2b = getBeamAllocation();
	vector<int> thatb2b = other.getBeamAllocation();
	int		maxBeamlets = thisb2b.size();
	for (int bl = 0; bl < maxBeamlets; bl++) {
		if (thisb2b[bl] != -1 && thatb2b[bl] != -1 && thisb2b[bl] != 999) {
			LOG_INFO_STR("Conflict in beamlets between observation " << obsID <<
						 " and " << other.obsID);
			LOG_DEBUG_STR("First conflicting beamlet: " << bl);
			return (true);
		}
	}

	return (false);	// no conflicts
}

//
// getRCUbitset(nrLBAs, nrHBAs, antennaSet): bitset
//
// Returns a bitset containing the RCU's requested by the observation.
// The can be te list specified in the receiverList or (when empty) just some
// standin dummmy purely based on the number of antennas.
//
bitset<MAX_RCUS> Observation::getRCUbitset(int nrLBAs, int nrHBAs, const string& anAntennaSet)
{
	// user defined set always overrules.
	if (RCUset.any()) {
		return (RCUset);
	}

	// HBA's in Core stations sometimes use half of the rcus.
	int	nrRCUS = 2 * ((anAntennaSet.find("LBA") == 0) ? nrLBAs : nrHBAs);
	
	// Set up the RCUbits for this antennaSet. Remember that we don't care here which of the three inputs is used.
	RCUset_t	tmpRCUset(RCUset);
	for (int rcu = 0; rcu < nrRCUS; rcu++) {	// check all bits
		tmpRCUset.set(rcu);
	}
	return (tmpRCUset);
}

//
// getBeamAllocation(stationname)
//
// Return station specific beamlet2beam vector.
//
vector<int> Observation::getBeamAllocation(const string& stationName) const
{
	vector<int>		b2b;

	// construct stationname if not given by user.
	string	station(stationName);
	if (station.empty()) {
		station = myHostname(false);
		char	lastChar(*(--(station.end())));
		if (lastChar == 'C' || lastChar == 'T') {
			station.erase(station.length()-1, 1);		// station.pop_back();
		}
	}
	if (!_isStationName(station)) {					// called on a non-station machine?
		return (b2b);									// return an empty vector
	}

	if (!itsHasDataslots) {
		return (beamlet2beams);	// return old mapping so it keeps working
	}

	// is DSL for this station available?
	string	fieldName = getAntennaFieldName(itsStnHasDualHBA);
	string	dsl(str(format("%s%s.DataslotList") % station % fieldName));
	string	rbl(str(format("%s%s.RSPBoardList") % station % fieldName));
	if (!itsDataslotParset.isDefined(dsl) || !itsDataslotParset.isDefined(rbl)) {
		LOG_ERROR_STR("No dataslots defined for " << station << antennaArray);
		return (b2b);
	}
	vector<int>	RSPboardList = itsDataslotParset.getIntVector(rbl,true);
	vector<int>	DataslotList = itsDataslotParset.getIntVector(dsl,true);

	ASSERTSTR (RSPboardList.size() == DataslotList.size(), "RSPBoardlist (" << RSPboardList << 
			") differs size of DataslotList(" << DataslotList << ") for station " << station);
	ASSERTSTR (RSPboardList.size() == itsBeamSlotList.size(), RSPboardList.size() << 
			" dataslot allocations, but beams specify " << itsBeamSlotList.size() << " for station " << station);

	// initialize arrays
	b2b = itsSlotTemplate;

	// fill with required information
	for (int i = RSPboardList.size()-1; i >= 0; --i) {
		int	idx = RSPboardList[i] * maxBeamletsPerRSP(bitsPerSample) + DataslotList[i];
		if (b2b[idx] != -1) {
			THROW (Exception, "beamlet " << i << " of beam " << itsBeamSlotList[i] << " clashes with beamlet of other beam(" << b2b[idx] << ")"); 
		}
		else {
			b2b[idx] = itsBeamSlotList[i];
		}
	}

	return (b2b);
}

//
// getBeamlets(beamNr, [stationName])
//
vector<int>	Observation::getBeamlets (uint beamIdx, const string&	stationName) const
{
	uint	parsetIdx = (dualMode && itsStnHasDualHBA) ? beamIdx/2 : beamIdx;
	string	fieldName = getAntennaFieldName(itsStnHasDualHBA, beamIdx);

	// construct stationname if not given by user.
	string	station(stationName);
	if (station.empty()) {
		station = myHostname(false);
		char	lastChar(*(--(station.end())));
		if (lastChar == 'C' || lastChar == 'T') {
			station.erase(station.length()-1, 1);		// station.pop_back();
		}
	}
		
	vector<int>	result;
	if (!_isStationName(station)) {					// called on a non-station machine?
		return (result);								// return an empty vector
	}

	if (!itsHasDataslots) {
		// both fields use the same beamlet mapping
		return (itsDataslotParset.getInt32Vector(str(format("Beam[%d].beamletList") % parsetIdx), vector<int32>(), true));	// true:expandable
	}

	// is DSL for this station available?
	// both fields have their own beamlet mapping
	string	dsl(str(format("%s%s.DataslotList") % station % fieldName));
	string	rbl(str(format("%s%s.RSPBoardList") % station % fieldName));
	if (!itsDataslotParset.isDefined(dsl) || !itsDataslotParset.isDefined(rbl)) {
		return (result);
	}
	vector<int>	RSPboardList = itsDataslotParset.getIntVector(rbl,true);
	vector<int>	DataslotList = itsDataslotParset.getIntVector(dsl,true);
	uint	nrEntries = itsBeamSlotList.size();
	for (uint i = 0; i < nrEntries; ++i) {
		if (itsBeamSlotList[i] == parsetIdx) {
			result.push_back(RSPboardList[i] * maxBeamletsPerRSP(bitsPerSample) + DataslotList[i]);
		}
	}
	return (result);
}


//
// TEMP HACK TO GET THE ANTENNAFIELDNAME
//
// Except for the beamIdx dependancy we should look in the antennaSet file.
//
string Observation::getAntennaFieldName(bool hasSplitters, uint32	beamIdx) const
{
	string	result;
	if (antennaSet.empty()) {
		result = antennaArray;
	}
	else {
		result = antennaSet;
	}

	if (result.find("LBA") == 0)  {
		return ("LBA");
	}
	
	if (!hasSplitters) {		// no splitter, always use all HBA
		return ("HBA");
	}

	// station has splitters
	if (result.substr(0,8) == "HBA_ZERO") 	return ("HBA0");
	if (result.substr(0,7) == "HBA_ONE") 	return ("HBA1");
	if (result.substr(0,10)== "HBA_JOINED")	return ("HBA");
	if (result.substr(0,8) == "HBA_DUAL")	return (beamIdx % 2 == 0 ? "HBA0" : "HBA1");
	return ("HBA");
}	

//
// getBeamName(beamidx): string
//
string Observation::getBeamName(uint32	beamIdx) const
{
	return (formatString("observation[%d]beam[%d]", obsID, beamIdx));
}
// there can only be one analogue beam
string Observation::getAnaBeamName() const
{
	return (formatString("observation[%d]anabeam", obsID));
}


//
// _isStationName(name)
//
bool Observation::_isStationName(const string&	hostname) const
{
	// allow AA999, AA999C and AA999T
	if (hostname.length() != 5 && hostname.length() != 6) 
		return (false);

	// We make a rough guess about the vality of the hostname.
	// If we want to check more secure we have to implement all allowed stationnames
	return (isalpha(hostname[0]) && isalpha(hostname[1]) &&
			isdigit(hostname[2]) && isdigit(hostname[3]) && isdigit(hostname[4]));
}

//
// _hasDataSlots
//
bool Observation::_hasDataSlots(const ParameterSet*	aPS) const
{
	ParameterSet::const_iterator	iter = aPS->begin();
	ParameterSet::const_iterator	end  = aPS->end();
	while (iter != end) {
		string::size_type	pos(iter->first.find("Dataslots."));
		// if begin found, what is after it?
		if (pos != string::npos && iter->first.find("Dataslots.DataslotInfo.") == string::npos) {	
			return _isStationName((iter->first.substr(pos+10,5)));
		}
		iter++;	// try next line
	}
	
	return (false);
}

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
	if (filterName == "LBA_30_80")		{ return(1); }

	// support of new names
	if (filterName == "LBA_10_70")		{ return(1); }
	if (filterName == "LBA_30_70")		{ return(1); }
	if (filterName == "LBA_10_90")		{ return(1); }
	if (filterName == "LBA_30_90")		{ return(1); }
	if (filterName == "HBA_110_190") 	{ return(2); }
	if (filterName == "HBA_170_230") 	{ return(3); }
	if (filterName == "HBA_210_250") 	{ return(3); }

	LOG_ERROR_STR ("filterselection value '" << filterName << 
											"' not recognized, using LBA_10_90");
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
    os << "stations     : " << stations << endl;
//    os << "stations     : "; writeVector(os, stations, ",", "[", "]"); os << endl;
    os << "antennaArray : " << antennaArray << endl;
    os << "antenna set  : " << antennaSet << endl;
    os << "receiver set : " << RCUset << endl;
    os << "sampleClock  : " << sampleClock << endl;
    os << "bits/sample  : " << bitsPerSample << endl;
    os << "filter       : " << filter << endl;
    os << "splitter     : " << (splitterOn ? "ON" : "OFF") << endl;
    os << "nyquistZone  : " << nyquistZone << endl << endl;

	os << "(Receivers)  : " << receiverList << endl;
	os << "Stations     : " << stationList << endl;
	os << "BLG nodes    : " << BGLNodeList << endl;
	os << "Storage nodes: " << storageNodeList << endl << endl;

    os << "nrBeams      : " << beams.size() << endl;
	for (size_t	b(0) ; b < beams.size(); b++) {
		os << "Beam[" << b << "].name       : " << beams[b].name << endl;
		os << "Beam[" << b << "].target     : " << beams[b].target << endl;
		os << "Beam[" << b << "].antennaSet : " << beams[b].antennaSet << endl;
		os << "Beam[" << b << "].momID      : " << beams[b].momID << endl;
		os << "Beam[" << b << "].subbandList: "; writeVector(os, beams[b].subbands, ",", "[", "]"); os << endl;
		os << "Beam[" << b << "].beamletList: "; writeVector(os, getBeamlets(b), ",", "[", "]"); os << endl;
		os << "nrPointings : " << beams[b].pointings.size() << endl;
		for (size_t p = 0; p < beams[b].pointings.size(); ++p) {
			const Pointing*		pt = &(beams[b].pointings[p]);
			os << formatString("Beam[%d].pointing[%d]: %f, %f, %s, %s\n", b, p, pt->angle1, pt->angle2, 
				pt->directionType.c_str(), to_simple_string(from_time_t(pt->startTime)).c_str());
		}
		os << "nrTABs      : " << beams[b].nrTABs << endl;
		os << "nrTABrings  : " << beams[b].nrTABrings << endl;
		os << "TABringsize : " << beams[b].TABringSize << endl;
		for (int t = 0; t < beams[b].nrTABs; ++t) {
			const TiedArrayBeam*	tab = &(beams[b].TABs[t]);
			os << formatString ("Beam[%d].TAB[%d]: %f, %f, %s, %f, %scoherent\n", b, t, tab->angle1, tab->angle2, tab->directionType.c_str(), tab->dispersionMeasure, (tab->coherent ? "" : "in"));
		}
	}
	os << "beamlet2beams   : "; writeVector(os, getBeamAllocation(), ",", "[", "]"); os << endl;

    os << "nrAnaBeams   : " << anaBeams.size() << endl;
	for (size_t	b(0) ; b < anaBeams.size(); b++) {
		os << "AnaBeam[" << b << "].name      : " << anaBeams[b].name << endl;
		os << "AnaBeam[" << b << "].antennaSet: " << anaBeams[b].antennaSet << endl;
		os << "AnaBeam[" << b << "].rank      : " << anaBeams[b].rank << endl;
		os << "nrPointings : " << anaBeams[b].pointings.size() << endl;
		for (uint p = 0; p < beams[b].pointings.size(); ++p) {
			const Pointing*		pt = &(anaBeams[b].pointings[p]);
			os << formatString("anaBeam[%d].pointing[%d]: %f, %f, %s\n", b, p, pt->angle1, pt->angle2, pt->directionType.c_str());
		}
	}

	return (os);
}


} // namespace LOFAR
