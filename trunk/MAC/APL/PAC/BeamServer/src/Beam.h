//#  ABSBeam.h: interface of the Beam class
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

#ifndef BEAM_H_
#define BEAM_H_

#include <lofar_config.h>
#include <APL/BS_Protocol/Pointing.h>
#include <APL/BS_Protocol/Beamlet2SubbandMap.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/CAL_Protocol/SpectralWindow.h>
#include <APL/CAL_Protocol/SubArray.h>
#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <time.h>
#include <AMCBase/Converter.h>

#include <queue>
#include <set>
#include <map>

#include <blitz/array.h>

#include "Beamlets.h"

namespace LOFAR {
  namespace BS {

// Class representing a single beam allocated by a client
// using a BEAMALLOC event.
class Beam {
public:

	// Default constructor
	// @param name String identifying this beam uniquely in the OTDB, used with
	// key-value logger as nodeid.
	// @param subarrayname The name of the subarray on which this beam is defined.
	// @param nsubbands The number of subbands of this beam.
	Beam(std::string name, std::string subarrayname, int nsubbands);

	// Default destructor.
	virtual ~Beam();

	// Allocate a new beam.
	// @param allocation Allocation of beamlet (indices) to subband (indices)
	// @param beamlets Allocate from this set of beamlets.
	// @param nsubbands The maximum number of subbands for this beam.
	// @return bool true if allocation successful, false if allocation failed
	// because specified beamlet has already been allocated or there is 
	// a range problem with the beamlet or subband indices.
	bool allocate(BS_Protocol::Beamlet2SubbandMap allocation, Beamlets& beamlets, int nsubbands);

	// Modify beam by chaning the subbands
	// of the beamlet2subband mapping. The lhs of the mapping
	// can not be changed.
	// @return bool true if modification succeeded, false if it failed
	// because beamlet set was changed or if there is a range problem
	// with the beamlet or subband indices.
	bool modify(BS_Protocol::Beamlet2SubbandMap allocation);

	// Get the allocation mapping for this beam.
	// @return Beamlet2SubbandMap the mapping from beamlet to subband for this beam.
	BS_Protocol::Beamlet2SubbandMap getAllocation() const;

	// @return Current pointing.
	inline BS_Protocol::Pointing getPointing() const { return m_pointing; }

	// Add a pointing to a beam.
	void addPointing(const BS_Protocol::Pointing& pointing);

	// Set the subarray (positions & rcu_index)
	void setSubarray(const CAL::SubArray& array);

	// Return a reference to the subarray for this beam.
	// @return reference to the subarray
	const CAL::SubArray& getSubarray() const { return m_array; }

	// setCalibration weights for the receivers
	void setCalibration(const CAL::AntennaGains& gains);

	// Get the current calibration values.
	const CAL::AntennaGains& getCalibration() const;

	// Log pointing information of this beam
	// to GET_CONFIG_PATH + "/indi_pipe" which should
	// be a named pipe. The data on this named pipe
	// is read by an INDI driver and passed on to KStars.
	//void logPointing(BS_Protocol::Pointing pointing);

	// Convert coordinates from the m_pointing_queue
	// to the local coordinate system, for times >= begintime
	// and begintime < begintime + m_compute_interval.
	// Converted coordinates are put on the m_coordinate_track
	// queue.
	// @param begintime First time of pointing to convert, this is typically
	// the last time the method was called. E.g.
	// @code
	// Timestamp begintime=lasttime;
	// lasttime.setNow();
	// lasttime += compute_interval; // compute_interval seconds ahead in time
	// for (beam in beams)
	// {
	//   // convert coordinate for next compute_interval seconds
	//   beam->calcNewTrack(begintime, compute_interval);
	// }
	// @endcode
	// @param compute_interval the interval for which pointings must be computed
	// @param conv The Converter object to use for coordinate conversions.
	// @return int 0 if successful, < 0 otherwise
	int calcNewTrack(RTC::Timestamp 	begintime, 
					 int 				compute_interval, 
					 AMC::Converter* 	conv);

	// Calculate the HBAdelays for the new track.
	void calculateHBAdelays(RTC::Timestamp					timestamp,
							AMC::Converter* 				conv,
							const blitz::Array<double,2>&	tileDeltas,
							const blitz::Array<double,1>&	elementDelays);

	// Return an array with the delays for HBA receivers of this beam.
	const blitz::Array<uint8,2>&	getHBAdelays() const;

	// Get converted time-stamped coordinates from the queue.
	// This method is called by the Beamlet class to get a priority
	// queue of coordinates.
	// @return array with the coordinates for the next period.
	const blitz::Array<double,2>& getLMNCoordinates() const;

	// Return the spectral window for this beam.
	const CAL::SpectralWindow& getSPW() const;

	// Get the name of the subarray on which this beam operates.
	std::string getSubarrayName() const { return m_subarrayname; }

	// Get beam name (can be used as key in key-value logger).
	std::string getName() const { return m_name; }

private:
	// Don't allow copying this object.
	Beam (const Beam&);            // not implemented
	Beam& operator= (const Beam&); // not implemented

	// Method to undo an allocation.
	void deallocate();

	//# --- datamembers ---

	// Name used as key in key-value logger.
	std::string 		m_name;

	// Name of the beam or subarray on which the beam is allocated.
	std::string 		m_subarrayname;

	// Allocation.
	BS_Protocol::Beamlet2SubbandMap m_allocation;

	// number of subbands
	int					m_nsubbands;

	/** current direction of the beam */
	BS_Protocol::Pointing m_pointing;

	// queue of future pointings as delivered by the user.
	std::priority_queue<BS_Protocol::Pointing> m_pointing_queue;

	// Current coordinate track in station local coordinates.
	// Two dimensional array of compute_interval x 3 (l,m,n) coordinates
	blitz::Array<double,2> m_lmns;  // l,m,n coordinates

	// Current coordinate (track of 1 element) in AZEL for HBA delay calculations.
	double					itsHBAazimuth;
	double					itsHBAelevation;

	// Array with the delays for the analog beamformer.
	// [max_number_rcus, elements in a HBA tile] = [192,16]
	blitz::Array<uint8,2>	itsHBAdelays;

	// Set of beamlets belonging to this beam.
	// It is a set because there should be no
	// duplicate beamlet instances in the set.
	std::set<Beamlet*> 	m_beamlets;

	// The antenna array.
	CAL::SubArray 		m_array;

	// The antenna gains.
	CAL::AntennaGains 	m_gains;

	// Private constants.
	static const int N_TIMESTEPS = 20; // number of timesteps to calculate ahead
};

//# -------------------- inline functions --------------------
//# getLMNCoordinates()
inline const blitz::Array<double,2>& Beam::getLMNCoordinates() const
{
	return (m_lmns);
}

//# getSPW()
inline const CAL::SpectralWindow& Beam::getSPW() const
{
	return (m_array.getSPW());
}

//# getHBAdelays()
inline const blitz::Array<uint8,2>&	Beam::getHBAdelays() const
{
	return (itsHBAdelays);
}

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAM_H_ */
