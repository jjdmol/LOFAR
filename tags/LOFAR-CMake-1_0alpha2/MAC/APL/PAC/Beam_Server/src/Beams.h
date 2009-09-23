//#  Beams.h: Collection of beam classes.
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

#ifndef BEAMS_H_
#define BEAMS_H_

#include <APL/BS_Protocol/Pointing.h>
#include <APL/BS_Protocol/Beamlet2SubbandMap.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/CAL_Protocol/SpectralWindow.h>
#include <APL/CAL_Protocol/SubArray.h>
#include <APL/CAL_Protocol/AntennaGains.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <time.h>
#include <AMCBase/Converter.h>

#include <queue>
#include <set>
#include <map>

#include <blitz/array.h>

#include "Beamlet.h"
#include "Beam.h"

namespace LOFAR {
  namespace BS {

// Factory class for Beam. This class manages the collection of Beams
// that are active in the BeamServer at a particular point in time.
class Beams {
public:
	// Create a collection of beams with subbands allocated from maxBeamletsEver.
	// @param maxBeamletsEver The maximum number of beamlets that can be allocated
	// @param maxSubbandsEver The maximum number of subbands that can be selected (valid
	//        subbands are 0 <= subbands < maxSubbandsEver.
	explicit Beams(int maxBeamletsEver, int maxSubbandsEver);

	/* destructor */
	~Beams();

	// Create a new beam.
	Beam* create(std::string 						name, 
				 std::string 						subarrayname, 
				 BS_Protocol::Beamlet2SubbandMap	allocation,
				 int								ringNr);

	// Set calibration handle for a beam
	void setCalibrationHandle(Beam* beam, CAL_Protocol::memptr_t handle);

	// Find calibration handle.
	CAL_Protocol::memptr_t findCalibrationHandle(Beam* beam) const;

	// Update gains
	bool updateCalibration(CAL_Protocol::memptr_t handle, CAL::AntennaGains& gains);

	// Check if a beam exists.
	bool exists(Beam* beam);

	// Destroy a beam. The beam is 'delete'ed.
	// @return bool true if beam has been found and deleted, false otherwise
	bool destroy(Beam* beam);

	// Calculate weights for all beamlet of all beams
	// for the specified number of time steps, the results are stored in
	// the weights array)
	void calculate_weights(RTC::Timestamp 							timestamp,
						   int 										compute_interval,
						   AMC::Converter* 							conv,
					   	   blitz::Array<std::complex<double>, 3>&	weights);

	// Calculate HBA-delays for the receivers of the HBA beams and 
	// send them to the RSPdriver.
	void calculateHBAdelays(RTC::Timestamp 					timestamp,
							int 							compute_interval,
							AMC::Converter* 				conv,
							const blitz::Array<double,2>&	tileDeltas,
							const blitz::Array<double,1>&		elementDelays);

	// send the HBA delays of all HBA beams.
	void sendHBAdelays(RTC::Timestamp				timestamp,
					   GCF::TM::GCFPortInterface&	port);

	// Return the combined beamlet to subband
	// mapping for all beams.
	BS_Protocol::Beamlet2SubbandMap getSubbandSelection(int	ringNr);

private:
	//# --- datamembers ---

	// Registry of active beams with mapping to calibration handle.
	std::map<Beam*, CAL_Protocol::memptr_t> 	m_beams;

	// Reverde map from handle to beam to find beam corresponding to
	// calibration update.
	std::map<CAL_Protocol::memptr_t, Beam*> 	m_handle2beam;

	// Collection of all beamlets;
	Beamlets					itsBeamletPool;

	// The maximum number of subbands.
	int							itsMaxSubbands;
};

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAMS_H_ */
