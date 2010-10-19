//#  AnalogueBeam.h: implementation of the AnalogueBeam class
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

#include <APL/RTCCommon/PSAccess.h>
#include "BeamServerConstants.h"
#include "AnalogueBeam.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace std;
using namespace RTC;

//
// AnalogueBeam(name, subarray, nrSubbands)
//
AnalogueBeam::AnalogueBeam(const string& 				name, 
						   const bitset<MAX_RCUS>&		rcuMask,
						   uint							rankNr) :
	Beam		(name, rcuMask),
	itsRankNr	(rankNr)
{}

//
// ~AnalogueBeam
//
AnalogueBeam::~AnalogueBeam()
{
}

// TODO CHANGE THIS TO IRTF
//
// calculateHBAdelays(timestamp, amcconverter, tileRelPosArray)
// result is stored in itsHBAdelays
//
void AnalogueBeam::calculateHBAdelays(RTC::Timestamp				timestamp,
							  const blitz::Array<double,2>&	tileDeltas,
							  const blitz::Array<double,1>&	elementDelays)
{
	LOG_DEBUG(formatString("current HBApointing=(%f,%f)",
							pointingAtTime(timestamp).angle0(),
							pointingAtTime(timestamp).angle1()));
	
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

