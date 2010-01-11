//#  Beam.h: interface of the Beam class
//#
//#  Copyright (C) 2002-2009
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
#include <Common/lofar_string.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Beamlet2SubbandMap.h>
#include <APL/IBS_Protocol/Pointing.h>
#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/CAL_Protocol/SubArray.h>

#include <queue>
#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

// Class representing a single beam allocated by a client
// using a BEAMALLOC event.
class Beam {
public:

	// Default constructor
	// @param name String identifying this beam uniquely in the OTDB, used with
	// key-value logger as nodeid.
	// @param antennaSet The name of the AntenneField on which this beam is defined.
	// @param allocation How the subbands of the beam are mapped tot the bemalets.
	// @param rcuMask The RCUs that participate in this beam.
	// @param ringNr The serdes segment the allocation is ment for.
	Beam(const string& 								name, 
		 const string& 								antenneSet, 
		 const IBS_Protocol::Beamlet2SubbandMap&	allocation, 
		 const bitset<MAX_RCUS>&					rcuMask,
		 uint 										ringNr);

	// Default destructor.
	virtual ~Beam();

	// Add a pointing to a beam.
	void addPointing(const IBS_Protocol::Pointing& pointing);

	// Get the allocation mapping for this beam.
	// @return Beamlet2SubbandMap the mapping from beamlet to subband for this beam.
	IBS_Protocol::Beamlet2SubbandMap& allocation() { return(itsBeamletAllocation); }

	// @return Current pointing.
	IBS_Protocol::Pointing currentPointing(const RTC::Timestamp& time);

	// Set the subarray (positions & rcu_index)
//	void setSubarray(const CAL::SubArray& array);

	// Return a reference to the subarray for this beam.
	// @return reference to the subarray
//	const CAL::SubArray& getSubarray() const { return itsSubArray; }

	// setCalibration weights for the receivers
//	void setCalibration(const CAL::AntennaGains& gains);

	// Get the current calibration values.
//	const CAL::AntennaGains& getCalibration() const;

	// Get the name of the subarray on which this beam operates.
	string antennaSetName() const { return (itsAntennaSet); }

	// Get beam name (unique name, can be used as key in key-value logger).
	string name() const { return (itsName); }

	// Get beam name (unique name, can be used as key in key-value logger).
	const bitset<MAX_RCUS>& rcuMask() const { return (itsRCUs); }

	// Get number of ringSegment
	int ringNr() const	{ return (itsRingNr); }

	// Set handle (=uniq ID) from the CalServer
	void  calibrationHandle(void	*handle) { itsCShandle = handle; }
	void* calibrationHandle()				 { return (itsCShandle); }

	void calculateHBAdelays(RTC::Timestamp					timestamp,
						    const blitz::Array<double,2>&	tileDeltas,
						    const blitz::Array<double,1>&	elementDelays);
private:
	// Don't allow copying this object.
	Beam (const Beam&);            // not implemented
	Beam& operator= (const Beam&); // not implemented

	// Method to undo an allocation.
	void deallocate();

	//# ----- DATAMEMBERS -----

	// Name used as key in key-value logger.
	string 		itsName;

	// Name of the subarray on which the beam is allocated.
	string 		itsAntennaSet;

	// Allocation.
	IBS_Protocol::Beamlet2SubbandMap 	itsBeamletAllocation;

	// RCUs participating in the beam
	bitset<MAX_RCUS>					itsRCUs;

	// ringSegment the beam is allocated on
	int									itsRingNr;

	// current direction of the beam
	IBS_Protocol::Pointing 				itsCurrentPointing;

	// queue of future pointings as delivered by the user.
	std::priority_queue<IBS_Protocol::Pointing>	itsPointingQ;

	// The antenna array.
//	CAL::SubArray 		itsSubArray;

	// calserver handle	
	// will become obsolete when new ITRF CalServer is used.
	void*		itsCShandle;

};

//# -------------------- inline functions --------------------

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAM_H_ */
