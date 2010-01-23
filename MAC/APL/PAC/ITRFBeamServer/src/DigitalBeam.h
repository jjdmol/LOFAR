//#  DigitalBeam.h: interface of the DigitalBeam class
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

#ifndef DIGITALBEAM_H_
#define DIGITALBEAM_H_

#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Beamlet2SubbandMap.h>
#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/CAL_Protocol/SubArray.h>
#include "Beam.h"

#include <queue>
#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

// Class representing a single beam allocated by a client
// using a BEAMALLOC event.
class DigitalBeam : public Beam {
public:

	// Default constructor
	// @param name String identifying this beam uniquely in the OTDB, used with
	// key-value logger as nodeid.
	// @param antennaSet The name of the AntenneField on which this beam is defined.
	// @param allocation How the subbands of the beam are mapped tot the bemalets.
	// @param rcuMask The RCUs that participate in this beam.
	// @param ringNr The serdes segment the allocation is ment for.
	DigitalBeam(const string& 							name, 
			    const string& 							antenneSet, 
			    const IBS_Protocol::Beamlet2SubbandMap&	allocation, 
			    const bitset<MAX_RCUS>&					rcuMask,
			    uint 									ringNr);

	// Default destructor.
	virtual ~DigitalBeam();

	// Get the allocation mapping for this beam.
	// @return Beamlet2SubbandMap the mapping from beamlet to subband for this beam.
	IBS_Protocol::Beamlet2SubbandMap& allocation() { return(itsBeamletAllocation); }

	// setCalibration weights for the receivers
//	void setCalibration(const CAL::AntennaGains& gains);

	// Get the current calibration values.
//	const CAL::AntennaGains& getCalibration() const;

	// Get the name of the subarray on which this beam operates.
	string antennaSetName() const { return (itsAntennaSet); }

	// Get number of ringSegment
	int ringNr() const	{ return (itsRingNr); }

	// Set handle (=uniq ID) from the CalServer
	void  calibrationHandle(void	*handle) { itsCShandle = handle; }
	void* calibrationHandle()				 { return (itsCShandle); }

private:
	// Don't allow copying this object.
	DigitalBeam (const DigitalBeam&);            // not implemented
	DigitalBeam& operator= (const DigitalBeam&); // not implemented

	// Method to undo an allocation.
	void deallocate();

	//# ----- DATAMEMBERS -----

	// Name of the subarray on which the beam is allocated.
	string 		itsAntennaSet;

	// Allocation.
	IBS_Protocol::Beamlet2SubbandMap 	itsBeamletAllocation;

	// ringSegment the beam is allocated on
	int									itsRingNr;

	// The antenna array.
//	CAL::SubArray 		itsSubArray;

	// calserver handle	
	// will become obsolete when new ITRF CalServer is used.
	void*		itsCShandle;

};

//# -------------------- inline functions --------------------

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* DIGITALBEAM_H_ */
