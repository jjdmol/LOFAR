//#  Beams.cc: implementation of the Beams class
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

#include <AMCBase/Converter.h>
#include <AMCBase/Position.h>

#include <APL/RTCCommon/PSAccess.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include "BeamServerConstants.h"
#include "Beams.h"

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
// Beams(nrBeamlets, nrSubbands)
//
Beams::Beams(int 	maxBeamletsEver, int 	maxSubbandsEver) : 
	itsBeamletPool(maxBeamletsEver), itsMaxSubbands(maxSubbandsEver)
{}

//
// ~Beams()
//
Beams::~Beams()
{
	for (map<Beam*,CAL_Protocol::memptr_t>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi) {
		delete bi->first;
	}
	m_beams.clear();
}

//
// create(nodename, subarrayname, allocation)
//
Beam* Beams::create(string nodeid, string subarrayname, Beamlet2SubbandMap allocation, int ringNr)
{
	Beam* beam = new Beam(nodeid, subarrayname, ringNr);

	if (!beam) {
		return(0);
	}

	if (!beam->allocate(allocation, itsBeamletPool, itsMaxSubbands)) {
		LOG_ERROR("Failed to allocate all required beamlets");
		delete beam;
		return (0);
	}

	// add to list of active beams with 0 calibration handle
	m_beams[beam] = 0;
	return(beam);
}

//
// setCalibrationHandle(beam, handle)
//
void Beams::setCalibrationHandle(Beam* beam, CAL_Protocol::memptr_t handle)
{
	m_handle2beam[handle] = beam;
	m_beams[beam]         = handle;
}

//
// findCalibrationHandle(beam)
//
CAL_Protocol::memptr_t Beams::findCalibrationHandle(Beam* beam) const
{
	map<Beam*,CAL_Protocol::memptr_t>::const_iterator it = m_beams.find(beam);

	if (it != m_beams.end()) {
		return (it->second);
	}

	return (0);
}

//
// updateCalibration(handle, gains)
//
bool Beams::updateCalibration(CAL_Protocol::memptr_t handle, CAL::AntennaGains& gains)
{
	map<CAL_Protocol::memptr_t,Beam*>::iterator it = m_handle2beam.find(handle);

	if ((it == m_handle2beam.end()) || (!it->second)) {
		LOG_WARN_STR("No calibration found for " << handle);
		return (false);
	}

	it->second->setCalibration(gains);
	return (true);
}

//
// exists(beam)
//
bool Beams::exists(Beam *beam)
{
	// if beam not found, return 0
	map<Beam*,CAL_Protocol::memptr_t>::iterator it = m_beams.find(beam);

	if (it == m_beams.end()) {
		return (false);
	}

	return (true);
}

//
// destroy(beam)
//
bool Beams::destroy(Beam* beam)
{
	// remove from handle2beam map
	for (map<CAL_Protocol::memptr_t,Beam*>::iterator it = m_handle2beam.begin();
									it != m_handle2beam.end(); ++it) {
		if (beam == it->second) {
			m_handle2beam.erase(it);
		}
	}

	// if beam not found, return false
	map<Beam*,CAL_Protocol::memptr_t>::iterator it = m_beams.find(beam);
	if (it != m_beams.end()) {
		delete(it->first);
		m_beams.erase(it);
		return (true);
	}

	return (false);
}

//
// calculate_weigths(time, interval, weights, AMCConverter)
//
void Beams::calculate_weights(Timestamp								 timestamp,
							  int									 compute_interval,
							  AMC::Converter*						 conv,
							  blitz::Array<std::complex<double>, 3>& weights)
{
	// iterate over all beams and fill m_lmns with new track of Pointings
	for (map<Beam*,CAL_Protocol::memptr_t>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi) {
		bi->first->calcNewTrack(timestamp, compute_interval, conv);
		LOG_DEBUG(formatString("current_pointing=(%f,%f)",
										bi->first->getPointing().angle0(),
										bi->first->getPointing().angle1()));
	}
	
	// beamlets can be part of multiple beams for first calculate the new tracks of
	// all beam and than loop over the beamlet s to calculate the weights.
	itsBeamletPool.calculate_weights(weights);
}

//
// getSubbandSelection()
//
Beamlet2SubbandMap Beams::getSubbandSelection(int ringNr)
{
	Beamlet2SubbandMap selection;
	for (map<Beam*,CAL_Protocol::memptr_t>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi) {
		if (bi->first->ringNr() == ringNr) {
			Beamlet2SubbandMap beammap = bi->first->getAllocation();
			selection().insert(beammap().begin(), beammap().end());
		}
	}

	return (selection);
}

//
// calculateHBAdelays(time, interval, converter, relTilePositions)
//
void Beams::calculateHBAdelays (RTC::Timestamp	 					timestamp,
								int 								compute_interval,
								AMC::Converter* 					conv,
								const blitz::Array<double,2>&		tileDeltas,
								const blitz::Array<double,1>&		elementDelays)
{
	// iterate over all beams and fill AzEl coordinates of HBA beams
	for (map<Beam*,CAL_Protocol::memptr_t>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi) {
		if (!bi->first->getSPW().isForHBA()) {
			LOG_DEBUG_STR("Beam " << bi->first->getName() << " is LBA");
			continue;
		}
		LOG_DEBUG_STR("Beam " << bi->first->getName() << " is HBA");

		//bi->first->calcNewTrack(timestamp, compute_interval, conv);
		//if itsHbaInterval < COMPUTE_INTERVAL) {
		//	bi->first->calcNewTrack(timestamp, 1, conv);
		//	LOG_DEBUG_STR("calculating new track for HBA");
		//}
		bi->first->calculateHBAdelays(timestamp, conv, tileDeltas, elementDelays);
	}
}

//
// sendHBAdelays(GCFPortInterface&	port)
//
void Beams::sendHBAdelays(RTC::Timestamp				time,
						  GCF::TM::GCFPortInterface&	port)
{
	// iterate over all beams and fill AzEl coordinates of HBA beams
	for (map<Beam*,CAL_Protocol::memptr_t>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi) {
		if (!bi->first->getSPW().isForHBA()) {
			continue;
		}
		RSPSethbaEvent	request;
		request.timestamp  = time;
		request.rcumask    = bi->first->getSubarray().getRCUMask();
		request.settings().resize(request.rcumask.count(), N_HBA_ELEM_PER_TILE);
		request.settings() = bi->first->getHBAdelays();

		LOG_INFO_STR("sending HBAdelays for beam " << bi->first->getName());
		port.send(request);
	}
}

