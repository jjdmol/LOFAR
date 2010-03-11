//#  AnaBeamMgr.h: implementation of the Beam class
//#
//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <ApplCommon/StationConfig.h>
#include <APL/APLCommon/AntennaSets.h>
#include <APL/APLCommon/AntennaPos.h>
#include <APL/RTCCommon/PSAccess.h>		// ParameterSet macros
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include "BeamServerConstants.h"
#include "AnaBeamMgr.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

#include <fcntl.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace std;
using namespace RTC;
using namespace APLCommon;

//
// AnaBeamMgr(name, subarray, nrSubbands)
//
AnaBeamMgr::AnaBeamMgr(uint		nrRCUsPerRing,
					   uint		nrRings) :
	itsRCUsPerRing		(nrRCUsPerRing),
	itsNrRings			(nrRings)
{
	ConfigLocator	cl;
	// load the HBA Deltas file
	getAllHBADeltas(cl.locate(GET_CONFIG_STRING("BeamServer.HBADeltasFile")));
	LOG_DEBUG("Loaded HBADeltas file");

	// load the HBA Delays file
	getAllHBAElementDelays(cl.locate(GET_CONFIG_STRING("BeamServer.HBAElementDelaysFile")));
	LOG_DEBUG("Loaded HBADelays file");

	// initialize size of array for HBA delay calculation
	StationConfig	sc;
	itsHBAdelays.resize(sc.nrRSPs * NR_RCUS_PER_RSPBOARD, N_HBA_ELEM_PER_TILE);

}

//
// ~AnaBeamMgr
//
AnaBeamMgr::~AnaBeamMgr()
{
	map<string, AnalogueBeam>::const_iterator	bIter = itsBeams.begin();
	map<string, AnalogueBeam>::const_iterator	bEnd  = itsBeams.end();
	while (bIter != bEnd) {
		deleteBeam(bIter->second.name());
		++bIter;
	}
}

//
// addBeam(anaBeam))
//
bool AnaBeamMgr::addBeam(const AnalogueBeam& beam)
{
	// already in admin?
	string		beamName(beam.name());
	map<string, AnalogueBeam>::const_iterator	iter = itsBeams.find(beamName);
	if (iter != itsBeams.end()) {
		LOG_ERROR_STR("Beam " << beamName << " already in my admistration");
		return (false);
	}	

	// remember the beam
	itsBeams[beamName] = beam;

	// add the pointing of this beam
	vector<Pointing>	pointings = beam.getAllPointings();
	size_t		nrPointings = pointings.size();
	LOG_DEBUG_STR("Beam "  << beamName << " has " << nrPointings << " analogue pointings");
	for (size_t p = 0; p < nrPointings; ++p) {
		PointingInfo	PI;
		PI.beam		= beam;
		PI.active	= false;
		PI.pointing = pointings[p];

		itsPointings.push_back(PI);
	}
	itsPointings.sort();
	return (true);
}

//
// deleteBeam(beam)
//
void AnaBeamMgr::deleteBeam(const string& beamName) 
{
	map<string, AnalogueBeam>::const_iterator	iter = itsBeams.find(beamName);
	if (iter == itsBeams.end()) {
		LOG_ERROR_STR("Beam " << beamName << " is not in my admistration, it cannot be deleted");
		return;
	}	
	itsBeams.erase(beamName);

	// delete all pointings
	list<PointingInfo>::iterator	Piter = itsPointings.begin();
	list<PointingInfo>::iterator	Pend  = itsPointings.end();
	while (Piter != Pend) {
		if (Piter->beam.name() == beamName) {
			Piter = itsPointings.erase(Piter);
		}
		else {
			++Piter;
		}
	}
	LOG_INFO_STR("Beam " << beamName << " removed from administration");
}

//
// addPointing(beamname, pointing)
//
bool AnaBeamMgr::addPointing(const string&	beamName, const Pointing&	newPt)
{
	// should be a known beam
	map<string, AnalogueBeam>::const_iterator	iter = itsBeams.find(beamName);
	if (iter == itsBeams.end()) {
		LOG_ERROR_STR("Beam " << beamName << " is not in my administration, pointing rejected");
		return (false);
	}	

	// add the pointing of this beam
	PointingInfo	PI;
	PI.beam		= iter->second;;
	PI.active	= false;
	PI.pointing = newPt;

	itsPointings.push_back(PI);
	itsPointings.sort();
	LOG_DEBUG_STR("Added analogue pointing for beam " << beamName  << ":" << newPt);

	return (true);
}


//
// activateBeams(timestamp)
//
void AnaBeamMgr::activateBeams(const Timestamp&	now)
{
	LOG_DEBUG_STR("activateBeams(" << now << ")");
	if (now < itsTargetTime) {
		LOG_ERROR_STR("activateBeams is called with a date in the past, results are not garanteed. " <<
						now << " < " << itsTargetTime);
	}
	itsTargetTime = now;

	// Note the pointings are in the order: rank, active, beamname, pointing time.
	list<PointingInfo>::iterator	iter = itsPointings.begin();
	list<PointingInfo>::iterator	end  = itsPointings.end();
	itsActiveRCUs.reset();
	while (iter != end) {
		string	beamName(iter->beam.name());						// handle everything per beam

		// Note: remember that the pointings/beams are already in order of importance. Whenever an active beam
		//       conflicts with a (more important) beam we handled before we are allowed to switch it off.
		bitset<MAX_RCUS>	conflictingRCUs = itsActiveRCUs;		// calc conflicting RCUs.
		conflictingRCUs &= iter->beam.rcuMask();
		if (conflictingRCUs.any() && iter->active) {				// if there is a conflict, switch it off.
			LOG_INFO_STR("Beam " << beamName << " is switched OFF");
			iter->active = false;
		}
		bool	beamIsActive(iter->active);							// remember state of this beam
		
		// delete old (expired) pointings of this beam
		while (iter != end && iter->beam.name() == beamName && iter->pointing.endTime() < now) {
			LOG_DEBUG_STR("Removing pointing " << beamName << ":" << iter->pointing);
			iter = itsPointings.erase(iter);
		}
		// reached end of this beam? restart our loop
		if ((iter == end) || (iter != end && iter->beam.name() != beamName))  {
			itsBeams.erase(beamName);
			LOG_INFO_STR("Beam " << beamName << " has ended");
			continue;
		}
		
		if (beamIsActive) {									// active beams should stay active.
			iter->active = true;							// make that pointing the active one
			itsActiveRCUs |= iter->beam.rcuMask();				// update occupied rcus
		}
		else {		// beam is not active try to activate it 
			// activate the beam when that is possible.
			if (conflictingRCUs.none() && iter->pointing.time() <= now) {
				iter->active = true;
				itsActiveRCUs |= iter->beam.rcuMask();					// update occupied rcus
				LOG_INFO_STR("Beam " << beamName << " is switched ON");
			}
		}
		
		// skip rest of the pointings of this beam
		while (iter != end && iter->beam.name() == beamName) {
			++iter;
		}
	} // while
}

//
// showAdmin()
//
void AnaBeamMgr::showAdmin() const
{
	LOG_DEBUG_STR("Registered beams:");
	map<string, AnalogueBeam>::const_iterator	bIter = itsBeams.begin();
	map<string, AnalogueBeam>::const_iterator	bEnd  = itsBeams.end();
	while (bIter != bEnd) {
		LOG_DEBUG_STR(bIter->first);
		++bIter;
	}

	LOG_DEBUG_STR("Registered Pointings:");
	list<PointingInfo>::const_iterator		pIter = itsPointings.begin();
	list<PointingInfo>::const_iterator		pEnd  = itsPointings.end();
	while (pIter != pEnd) {
		LOG_DEBUG_STR(pIter->beam.name() << "{" << pIter->beam.rankNr() << "}: " << (pIter->active ? "ON" : "off") << "==>" <<  pIter->pointing);
		++pIter;
	}
}


//
// calculateHBAdelays(timestamp, amcconverter, tileRelPosArray)
// result is stored in itsHBAdelays
//
void AnaBeamMgr::calculateHBAdelays(RTC::Timestamp	targetTime, J2000Converter&	aJ2000Conv)
{
	// make sure that we use the right pointings.
	if (targetTime != itsTargetTime) {
		activateBeams(targetTime);
	}

	// Are there any beams and pointings left?
	if (itsBeams.empty() || itsPointings.empty()) {
		itsTargetTime = Timestamp(0,0);
		return;
	}

	showAdmin();
	LOG_INFO_STR("Calculating HBAdelays for time " << targetTime);

	itsHBAdelays(Range::all(), Range::all()) = 0;
	itsTargetTime = targetTime;

	// When have to do the calculations on the HBA, HBA0 and HBA1 field. (at least we have to check those fields).
	// To prevent needless conversions we first scan the pointings for each field to see if something active
	// is in that field. If so, we do the rest.

	for (int fieldNr = 0; fieldNr < 3; ++fieldNr) {
		string	fieldName;
		switch(fieldNr) {
			case 0:	fieldName = "HBA";	break;
			case 1:	fieldName = "HBA0";	break;
			case 2:	fieldName = "HBA1";	break;
		}

		list<PointingInfo>::const_iterator		pIter = itsPointings.begin();
		list<PointingInfo>::const_iterator		pEnd  = itsPointings.end();
		while(pIter != pEnd && (!pIter->active || globalAntennaSets()->antennaField(pIter->beam.antennaSetName()) != fieldName)) {
			++pIter;
		}
		if (pIter == pEnd) {
			LOG_DEBUG_STR("No active beams in the " << fieldName << " field");
			continue;		// try next field
		}

		// we found an active beam in this HBA field, calculate the delays for all 
		// active beams in this field

		// Get geographical location of antennaField in ITRF
		blitz::Array<double, 1> fieldCentreITRF = globalAntennaPos()->Centre(fieldName);
		LOG_DEBUG_STR("ITRF position antennaField " << fieldName << ": " << fieldCentreITRF);

		LOG_DEBUG_STR("itsTileRelPos = " << itsTileRelPos);

		// Calc length of vectors of rel. positions of the tile elements
		blitz::Array<double, 1>		tileRelLength(itsTileRelPos.extent(firstDim));
		for (int i = 0; i < itsTileRelPos.extent(firstDim); i++) {
			tileRelLength(i) = sqrt((itsTileRelPos(i,0) * itsTileRelPos(i,0)) +
									 (itsTileRelPos(i,1) * itsTileRelPos(i,1)) +
									 (itsTileRelPos(i,2) * itsTileRelPos(i,2)));
		}
		LOG_DEBUG_STR("tileRelLength:" << tileRelLength);
		
		// for all active pointings in this antennafield
		while (pIter != pEnd) {
			// must be of the same antenna field.
			LOG_DEBUG_STR(pIter->beam.name() << "@" << pIter->beam.antennaSetName());
			if (!pIter->active || globalAntennaSets()->antennaField(pIter->beam.antennaSetName()) != fieldName) {
				++pIter;
				continue;
			}
			// Get the right pointing
			Pointing	currentPointing = pIter->pointing;
			LOG_DEBUG_STR("current analogue Pointing = " << currentPointing);
			if (currentPointing.getType() == "NONE") {
				++pIter;
				continue;
			}

			blitz::Array<double,2>	sourceJ2000xyz;		// [rcu, xyz]
			blitz::Array<double,2>	curPoint(1,2);		// [1, angles]
			curPoint(0,0) = currentPointing.angle0();
			curPoint(0,1) = currentPointing.angle1();

			if (!aJ2000Conv.doConversion(currentPointing.getType(), curPoint, fieldCentreITRF, targetTime, sourceJ2000xyz)) {
				LOG_FATAL_STR("Conversion of source to J2000 failed for beam " << pIter->beam.name());
				++pIter;
				continue;	// try next pointing
			}
			LOG_DEBUG_STR("sourceJ2000xyz:" << sourceJ2000xyz);

			blitz::Array<double, 2>	tileRelPosJ2000;
			if (!aJ2000Conv.doConversion("ITRF", itsTileRelPos, fieldCentreITRF, targetTime, tileRelPosJ2000)) {
				LOG_FATAL_STR("Conversion of deltas to J2000 failed for beam " << pIter->beam.name());
				++pIter;
				continue;	// try next pointing
			}
			tileRelPosJ2000 = tileRelPosJ2000(tensor::i, tensor::j) * tileRelLength(tensor::i);
			LOG_DEBUG_STR("tileRelPosJ2000:" << tileRelPosJ2000);

			bitset<MAX_RCUS>	RCUallocation(pIter->beam.rcuMask());
			LOG_DEBUG_STR("Rcuallocation: " << RCUallocation);
			for (int rcu = 0; rcu < MAX_RCUS; rcu++) {
				if (!RCUallocation.test(rcu)) {			// all RCUS switched on in LBA/HBA mode
					continue;
				}

				for (int element = 0; element < N_HBA_ELEM_PER_TILE; ++ element) {
					// calculate tile delay.
					double	delay = ( (sourceJ2000xyz(0,0) * tileRelPosJ2000(element,0)) +
									  (sourceJ2000xyz(0,1) * tileRelPosJ2000(element,1)) +
									  (sourceJ2000xyz(0,2) * tileRelPosJ2000(element,2)) ) / SPEED_OF_LIGHT_MS;
					
					// signal front stays in the middle of the tile
					delay += itsMeanElementDelay;

					LOG_DEBUG_STR("antenna="<<rcu/2 <<", pol="<<rcu%2 <<", element="<<element  <<", delay("<<rcu<<","<<element<<")="<<delay);
					// calculate approximate DelayStepNr
					int delayStepNr = static_cast<int>(delay / 0.5E-9);
					
					// look for nearest matching delay step in range "delayStepNr - 2 .. delayStepNr + 2"
					double minDelayDiff = fabs(delay - itsDelaySteps(delayStepNr));
					double difference;
					int minStepNr = delayStepNr;
					for (int i = (delayStepNr - 2); i <= (delayStepNr + 2); i++){
						if (i == delayStepNr) 
							continue; // skip approximate nr
						difference = fabs(delay - itsDelaySteps(i));
						if (difference < minDelayDiff)	{
							minStepNr = i;
							minDelayDiff = difference;
						}
					}
					delayStepNr = minStepNr;
					
					// range check for delayStepNr, max. 32 steps (0..31) 
					if (delayStepNr < 0)  delayStepNr = 0;
					if (delayStepNr > 31) delayStepNr = 31;
						
					// bit1=0.25nS(not used), bit2=0.5nS, bit3=1nS, bit4=2nS, bit5=4nS, bit6=8nS 	
					itsHBAdelays(rcu,element) = (delayStepNr * 4) + (1 << 7); // assign
				} // for element
			} // rcus
			++pIter;
		} // pointings
	} // antennafield
}

void AnaBeamMgr::sendHBAdelays(GCF::TM::GCFPortInterface&	port)
{
	RSPSethbaEvent  request;
	request.timestamp  = itsTargetTime;
	request.rcumask    = itsActiveRCUs;
	// note: the RSPDriver expects settings for the active RCU's only.
	// TODO: CHANGE THIS SO WE CAN POINT THE UNUSED TILES TO ZENITH
	request.settings().resize(itsActiveRCUs.count(), N_HBA_ELEM_PER_TILE);
	int nrRCUs = itsActiveRCUs.size(); 
	int	requestIdx(0);
	for (int r = 0; r < nrRCUs; r++) {
		if (itsActiveRCUs.test(r)) {
			request.settings()(requestIdx) = itsHBAdelays(r);
			requestIdx++;
		}
	}

	LOG_INFO("sending HBAdelays");
	port.send(request);

	for(int i = 0; i < requestIdx; i++) {
		ostringstream	os;
		for (int j = 0; j < N_HBA_ELEM_PER_TILE; j++) {
			os.width(4);
			os << (int) request.settings()(i, j);
		}
		LOG_DEBUG_STR(os.str());
	}

}

// --------------- READ CONFIGURATIONFILES ---------------

//
// getAllHBADeltas 
//
void AnaBeamMgr::getAllHBADeltas(const string& filename)
{
	ifstream	itsFile;
	string 		itsName;

	LOG_DEBUG_STR("Trying to read the relative HBA element positions from file " << filename);
	// open new file
	if (itsFile.is_open()) {
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	if (!itsFile.good()) {
		itsFile.close();
		return;
	}

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	getline(itsFile, itsName); // read name or comment
	while (itsName[0] == '#') {
		LOG_DEBUG_STR("HBADeltas comment = " << itsName);
		getline(itsFile, itsName); // read name or comment
	}
	
	if ("" == itsName) {
		itsFile.close();
		return;
	}
	
	LOG_DEBUG_STR("HBADeltas name = " << itsName);
	
	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsTileRelPos; // read HBA deltas array
	LOG_DEBUG_STR("HBADeltas = " << itsTileRelPos);

	LOG_INFO_STR("Relative HBA element positions read from file " << filename);
}

//
// getAllHBAElementDelays 
//
void AnaBeamMgr::getAllHBAElementDelays(const string& filename)
{
	ifstream	itsFile;
	string		itsName;

	LOG_DEBUG_STR("Trying to read the HBA element delay steps  from file " << filename);

	// open new file
	if (itsFile.is_open()) { 
		itsFile.close();
	}
	itsFile.open(filename.c_str());

	if (!itsFile.good()) {
		itsFile.close();
		return;
	}

	// The file may have comment lines at the top, starting with '#'
	// These must be skipped
	getline(itsFile, itsName); // read name or comment
	while (itsName[0] == '#') {
		LOG_DEBUG_STR("HBA ElementDelays comment = " << itsName);
		getline(itsFile, itsName); // read name or comment
	}
	
	if ("" == itsName) {
		itsFile.close();
		return;
	}

	LOG_DEBUG_STR("HBA ElementDelays Name = " << itsName);
	
	// Now comment lines are skipped, so we can read the full array.
	itsFile >> itsDelaySteps; // read HBA element delays array
	//itsDelaySteps *= 1E-9; // convert from nSec to Secs
	LOG_DEBUG_STR("HBA ElementDelays = " << itsDelaySteps);

	LOG_INFO_STR("HBA element delay steps read from file " << filename);
	itsMeanElementDelay = blitz::mean(itsDelaySteps);
	LOG_INFO_STR("mean ElementDelay = " << itsMeanElementDelay);
}

#if 0
//
// TODO CHANGE THIS TO IRTF
//
// calculateHBAdelays(timestamp, amcconverter, tileRelPosArray)
// result is stored in itsHBAdelays
//
void Beam::calculateHBAdelays(RTC::Timestamp				timestamp,
							  const blitz::Array<double,2>&	tileDeltas,
							  const blitz::Array<double,1>&	elementDelays)
{
	LOG_DEBUG(formatString("current HBApointing=(%f,%f)",
							currentPointing(timestamp).angle0(),
							currentPointing(timestamp).angle1()));
	
	// calculate the mean of all posible delays to hold signal front in the midle of a tile
	double meanElementDelay = blitz::mean(elementDelays);
	LOG_DEBUG_STR("mean ElementDelay = " << meanElementDelay << " Sec"); 

#if DOEN_WE_LATER
	// Calculate the current values of AzEl.
	Pointing	curPointing(itsCurrentPointing);
	curPointing.setTime(timestamp);
  // Get geographical location of subarray in WGS84 radians/meters
	blitz::Array<double, 1> loc = itsSubArray.getGeoLoc();
	Position location((loc(0) * M_PI) / 180.0,
					  (loc(1) * M_PI) / 180.0,
					   loc(2), Position::WGS84);
	// go to lmn coordinates
//	Pointing lmn	  = curPointing.convert(conv, &location, Pointing::LOFAR_LMN);
	double itsHBA_L   = lmn.angle0();
	double itsHBA_M   = lmn.angle1();
	
	// maybe not needed, but this code limits the max. posilble delay
	if ((itsHBA_L*itsHBA_L + itsHBA_M*itsHBA_M) > 1) {
		double modus = sqrt(itsHBA_L*itsHBA_L + itsHBA_M*itsHBA_M);
		itsHBA_L = itsHBA_L / modus;
		itsHBA_M = itsHBA_M / modus;
	}
	
	LOG_INFO_STR("current HBA-pointing lmn=(" << itsHBA_L << "," << itsHBA_M << ") @" << curPointing.time());

	// get position of antennas
	// note: pos[antennes, polarisations, coordinates]
	const Array<double,3>& pos  = itsSubArray.getAntennaPos();
	int nrAntennas = pos.extent(firstDim);
	int nrPols     = pos.extent(secondDim);
	LOG_DEBUG_STR("SA.n_ant="<<nrAntennas <<",SA.n_pols="<<nrPols);
	LOG_DEBUG_STR("antennaPos="<<pos);
	
	// get contributing RCUs
	CAL::SubArray::RCUmask_t		rcuMask(itsSubArray.getRCUMask());
	LOG_DEBUG_STR("rcumask="<<rcuMask);
	
	itsHBAdelays.resize(rcuMask.count(), MEPHeader::N_HBA_DELAYS);
	itsHBAdelays = 0;	// set all delays to 0
	
	int		localrcu  = 0;
	int		globalrcu = 0;
	for (globalrcu = 0; globalrcu < MEPHeader::MAX_N_RCUS; globalrcu++) {
		//LOG_DEBUG_STR("globalrcu=" << globalrcu);
		if (!rcuMask.test(globalrcu)) {
			continue;
		}
		// RCU is in RCUmask, do the calculations
		int	antenna(globalrcu/2);
		int pol	   (globalrcu%2);
		for (int element = 0; element < MEPHeader::N_HBA_DELAYS; element++) {
					
			// calculate tile delay
			double	delay =
				( (itsHBA_L * tileDeltas(element,0)) + 
				  (itsHBA_M * tileDeltas(element,1)) ) / 
					SPEED_OF_LIGHT_MS;
			
			// signal front stays in midle of tile
			delay += meanElementDelay;
			
			LOG_DEBUG_STR("antenna="<<antenna <<", pol="<<pol <<", element="<<element  <<", delay("<<localrcu<<","<<element<<")="<<delay);
			
			// calculate approximate DelayStepNr
			int delayStepNr = static_cast<int>(delay / 0.5E-9);
			
			// look for nearest matching delay step in range "delayStepNr - 2 .. delayStepNr + 2"
			double minDelayDiff = fabs(delay - elementDelays(delayStepNr));
			double difference;
			int minStepNr = delayStepNr;
			for (int i = (delayStepNr - 2); i <= (delayStepNr + 2); i++){
				if (i == delayStepNr) continue; // skip approximate nr
				difference = fabs(delay - elementDelays(i));
				if (difference < minDelayDiff)	{
					minStepNr = i;
					minDelayDiff = difference;
				}
			}
			delayStepNr = minStepNr;
			
			// range check for delayStepNr, max. 32 steps (0..31) 
			if (delayStepNr < 0) delayStepNr = 0;
			if (delayStepNr > 31) delayStepNr = 31;
				
			// bit1=0.25nS(not used), bit2=0.5nS, bit3=1nS, bit4=2nS, bit5=4nS, bit6=8nS 	
			itsHBAdelays(localrcu,element) = (delayStepNr * 4) + (1 << 7); // assign
		} // elements in tile
		localrcu++;	 // globalrcu
	}
		
	// temporary array needed to LOG "itsHBAdelays"
	blitz::Array<int,2>	HBAdelays;
	HBAdelays.resize(rcuMask.count(), MEPHeader::N_HBA_DELAYS);
	
	HBAdelays = itsHBAdelays + 0; // copy to print values (uint8 are handled as chars)
	LOG_DEBUG_STR("itsHBAdelays Nr = " << HBAdelays);
#endif
}
#endif

