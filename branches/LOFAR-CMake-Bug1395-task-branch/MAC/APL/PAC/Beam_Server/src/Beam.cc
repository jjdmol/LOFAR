//#  ABSBeam.h: implementation of the Beam class
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

#include <AMCBase/Converter.h>
#include <AMCBase/Position.h>

#include <APL/RTCCommon/PSAccess.h>
#include "BeamServerConstants.h"
#include "Beam.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

#include <fcntl.h>

using namespace blitz;
using namespace LOFAR;
using namespace AMC;
using namespace BS;
using namespace BS_Protocol;
using namespace std;
using namespace RTC;

//
// Beam(name, subarray, nrSubbands)
//
Beam::Beam(string name, string subarrayname, int ringNr) :
	m_name			(name), 
	m_subarrayname	(subarrayname),
	itsRingNr		(ringNr)
{}

//
// ~Beam
//
Beam::~Beam()
{
	deallocate();

	// TODO: free beamlets
}

//
// allocate(beamletAllocation, beamletPool, nrSubbands)
//
// Note: B2Smap = map <beamletnr, subbandnr>
//
bool Beam::allocate(Beamlet2SubbandMap allocation, Beamlets& beamlets, int nrSubbands)
{
	// check mapsize with number of subbands
	if (allocation().size() == 0 || allocation().size() > (unsigned)nrSubbands) {
		LOG_ERROR_STR("Need " << nrSubbands << " slots for mapping the beamlets, only " 
							  << allocation().size() << " available");
		return (false);
	}

	m_allocation = allocation;	// copy allocation
	m_beamlets.clear();			// clear the beamlet collection just to be sure

	// create a Beamlet object for every Beamletnr in the Beamlets array
	for (map<uint16,uint16>::iterator it = m_allocation().begin();
									  it != m_allocation().end(); ++it) {
		//																v--- beamletnumber
		Beamlet* beamlet = beamlets.get((itsRingNr * LOFAR::MAX_BEAMLETS) + (int)it->first);		// create or get entry
		if (!beamlet) {
			goto failure;
		}

		m_beamlets.insert(beamlet); //v--- subbandnr
		if (beamlet->allocate(*this, it->second, nrSubbands) < 0) {
			goto failure;
		}
	} // for

	return (true);

failure:
	// cleanup on failure
	deallocate();
	LOG_ERROR("Overlap in beamlets, no allocation possible");
	return (false);
}

//
// modify(BeamletAllocation)
//
bool Beam::modify(BS_Protocol::Beamlet2SubbandMap allocation)
{
	// new and old size should match
	if (allocation().size() != m_allocation().size()) {
		LOG_ERROR ("The number of beamlets may no be changed during reallocation");
		return (false);
	}

	// check that beamletnumbers match
	map<uint16,uint16>::iterator it1;
	map<uint16,uint16>::iterator it2;
	for (it1 = allocation().begin(), it2 = m_allocation().begin();
										it1 != allocation().end(); ++it1, ++it2) {
		if ((*it1).first != (*it2).first) {
			LOG_ERROR("Overlap in beamlets, no re-allocation possible");
			return (false);
		}
	}

	// all ok, replace m_allocation by allocation
	m_allocation = allocation;

	return (true);
}

//
// getAllocation()
//
Beamlet2SubbandMap Beam::getAllocation() const
{
	return (m_allocation);
}

//
// deallocate()
//
void Beam::deallocate()
{
	// release beamlets
	for (set<Beamlet*>::iterator bl = m_beamlets.begin();
								 bl != m_beamlets.end(); ++bl) {
		(*bl)->deallocate();
	}
	m_beamlets.clear();

	// reset pointing to zenith
	m_pointing = Pointing();

	// clear the pointing queue
	while (!m_pointing_queue.empty()) {
		m_pointing_queue.pop();
	}

	// clear allocation
	m_allocation().clear();
}

//
// addPointing(pointing)
//
void Beam::addPointing(const Pointing& pointing)
{
	m_pointing_queue.push(pointing);
}

//
// setSubarray(array)
//
void Beam::setSubarray(const CAL::SubArray& array)
{
	m_array = array;
}

//
// setCalibration(gains)
//
void Beam::setCalibration(const CAL::AntennaGains& gains)
{
	m_gains = gains;
}

//
// getCalibration()
//
const CAL::AntennaGains& Beam::getCalibration() const
{
	return (m_gains);
}

#if 0
static int setNonblocking(int fd)
{
  int flags;

  /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
  /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    flags = 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  /* Otherwise, use the old way of doing it */
  flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif
}     

//
// logPointing(pointing)
//
void Beam::logPointing(Pointing pointing)
{
  double hh, mm, ss, deg, degmm, degss;
  static FILE* pipe = 0;
  ConfigLocator cl;
  static string pipename = cl.locate("/indi_pipe");
  int retry = 0;

  if (GET_CONFIG("BeamServer.DISABLE_INDI", i) == 0) {
    /**
     * Write RaDec coordinates to the named pipe.
     * If there is no-one listening on the named pipe it
     * will eventually block which we prevent by making the filedescriptor
     * non-blocking.
     * When a write (fprintf) to the named-pipe fails, assume that the pipe
     * has been closed by the other end and re-open it and write the value again.
     */
    do {
      /* open pipe if not already open */
      if (!pipe) {
	pipe = fopen(pipename.c_str(), "w+");
	if (!pipe) {
	  LOG_WARN_STR("Could not open '" << pipename << "'");
	  return;
	}
	setNonblocking(fileno(pipe));
      }

      /*
       * If failed to write, closed the pipe and retry by
       * re-opening the pipe (at the beginning of the while loop.
       */
      if (fprintf(pipe, "%s %d %lf %lf\n",
		  getName().c_str(), 0, pointing.angle0(), pointing.angle1()) < 0) {
	fclose(pipe);
	pipe = 0;
      }
    }
    while (retry++ < 5);
  }

  hh = (pointing.angle0() * 180.0 / M_PI) / 15.0;
  mm = (hh - floor(hh)) * 60;
  ss = (mm - floor(mm)) * 60;
  hh = floor(hh);
  mm = floor(mm);
  
  deg = pointing.angle1() * 180.0 / M_PI;
  degmm = (deg - floor(deg)) * 60;
  degss = (degmm - floor(degmm)) * 60;
  deg = floor(deg);
  degmm = floor(degmm);
  if (deg > 90.0) deg = 360.0 - deg;

  LOG_INFO_STR("RA=" << hh << "h " << mm << "m " << ss << "s");
  LOG_INFO_STR("DEC=" << deg << "deg " << degmm << "' " << degss << "''");
      
  if (pipe) fflush(pipe);
}
#endif

//
// calcNewTrack (begintime, interval, amcConverter)
//
// Note: for LBA antenna's every second a new pointing must be send to the
// RSPboards. For HBA however the update interval is about 20 seconds so we
// only have to calculate one pointing.
//
int Beam::calcNewTrack(RTC::Timestamp	begintime, 
					   int				compute_interval, 
					   AMC::Converter*	conv)
{
	int		nrPts2Calc = compute_interval;

	// remember last pointing from previous call
	Pointing current_pointing = m_pointing;

	Pointing	track[nrPts2Calc];	// array with future directions
	bool		pset [nrPts2Calc];	// track element calculated or not
	for (int i = 0; i < nrPts2Calc; i++) {
		pset[i] = false;
	}

	// reset lmns array
	m_lmns.resize(nrPts2Calc, 3);
	m_lmns = 0.0;

	LOG_DEBUG_STR("computing track for interval [" << begintime 
					<< " : " << begintime + (long)(compute_interval - 1) << "]");

	// for all (user) pointings in the queue
	while (!m_pointing_queue.empty()) {
		Pointing pointing = m_pointing_queue.top();
		LOG_DEBUG_STR("Process pointing for time " << pointing.time());

		// Deadline missed?
		if (pointing.time() < begintime) {
			// print warning but execute the command anyway as soon as possible.
			LOG_WARN_STR("Deadline missed by " << (begintime.sec()-pointing.time().sec())
				<< " seconds (" << pointing.angle0() << "," << pointing.angle1() << ") "
				<< "@ " << pointing.time());

			pointing.setTime(begintime);
		}
		// note: pointing.time() is now >= begintime

		// within this period?
		if (pointing.time() < begintime + (long)compute_interval) {
			m_pointing_queue.pop(); // remove from queue because pointing
									// will be stored in m_pointing.

			// insert converted LM coordinate at correct 
			// position in the m_lmns array
			register int tsec = pointing.time().sec() - begintime.sec();

			if ( (tsec < 0) || (tsec >= compute_interval) ) {
				LOG_ERROR_STR("\ninvalid pointing time" << pointing.time());
				continue;
			}

			track[tsec] = pointing;
			pset[tsec]  = true;

			m_pointing = pointing; // remember current pointing
		} else {
			// pointing starts after calculation period
			break;
		}
	}

	// Get geographical location of subarray in WGS84 radians/meters
	blitz::Array<double, 1> loc = getSubarray().getGeoLoc();
	Position location((loc(0) * M_PI) / 180.0, 
					  (loc(1) * M_PI) / 180.0,
					   loc(2), Position::WGS84);
	LOG_DEBUG_STR("Geographical location " << loc);

	// track array will have unset values because
	// there are not necessarily pointings at each second
	// or if the pointingqueue was empty.
	// these holes are fixed up in this loop
	// and each pointing is converted to LMN coordinates
	for (int t = 0; t < nrPts2Calc; ++t) {
		if (!pset[t]) {
			track[t] = current_pointing;
		} else {
			current_pointing = track[t];
		}

		/* set time and convert to LMN */
		track[t].setTime(begintime + (long)t);
		Pointing lmn  = track[t].convert(conv, &location, Pointing::LOFAR_LMN);
		/* store in m_lmns and calculate normalized n-coordinate */
		m_lmns(t,0) = lmn.angle0();
		m_lmns(t,1) = lmn.angle1();
		m_lmns(t,2) = ::sqrt(1.0 - ((m_lmns(t,0) * m_lmns(t,0)) + 
									(m_lmns(t,1) * m_lmns(t,1))));

#if 0
		if ((fabs(m_lmns(t,0)) > 1.0 + eps) || (fabs(m_lmns(t,1)) > 1.0 + eps)) {

			LOG_WARN("\nl or m coordinate out of range -1.0 < l < 1.0, setting to (l,m) to (0.0, 0.0)\n");
			m_lmns(t,0) = 0.0;
			m_lmns(t,1) = 0.0;
			m_lmns(t,2) = 1.0;
		}
#endif

		LOG_DEBUG_STR(formatString("direction=(%f,%f,%f) @ ",
						m_lmns(t,0), m_lmns(t,1), m_lmns(t,2)) << track[t].time());
	} // for pointings in compute interval

	LOG_DEBUG_STR("Current pointing (" << current_pointing.angle0() << ", " 
									   << current_pointing.angle1() <<
										") @ time " << current_pointing.time());
	//logPointing(current_pointing);

	return (0);
}

//
// calculateHBAdelays(timestamp, amcconverter, tileRelPosArray)
// result is stored in itsHBAdelays
//
void Beam::calculateHBAdelays(RTC::Timestamp				timestamp,
							  AMC::Converter*				conv,
							  const blitz::Array<double,2>&	tileDeltas,
							  const blitz::Array<double,1>&	elementDelays)
{
	LOG_DEBUG(formatString("current HBApointing=(%f,%f)",
							getPointing().angle0(),
							getPointing().angle1()));
	
	// calculate the mean of all posible delays to hold signal front in the midle of a tile
	double meanElementDelay = blitz::mean(elementDelays);
	LOG_DEBUG_STR("mean ElemnetDelay = " << meanElementDelay << " Sec"); 


	// Calculate the current values of AzEl.
	Pointing	curPointing(getPointing());
	curPointing.setTime(timestamp);
  // Get geographical location of subarray in WGS84 radians/meters
	blitz::Array<double, 1> loc = getSubarray().getGeoLoc();
	Position location((loc(0) * M_PI) / 180.0,
					  (loc(1) * M_PI) / 180.0,
					   loc(2), Position::WGS84);
	// go to lmn coordinates
	Pointing lmn	  = curPointing.convert(conv, &location, Pointing::LOFAR_LMN);
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
	const Array<double,3>& pos  = getSubarray().getAntennaPos();
	int nrAntennas = pos.extent(firstDim);
	int nrPols     = pos.extent(secondDim);
	LOG_DEBUG_STR("SA.n_ant="<<nrAntennas <<",SA.n_pols="<<nrPols);
	LOG_DEBUG_STR("antennaPos="<<pos);
	
	// get contributing RCUs
	CAL::SubArray::RCUmask_t		rcuMask(getSubarray().getRCUMask());
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
}

