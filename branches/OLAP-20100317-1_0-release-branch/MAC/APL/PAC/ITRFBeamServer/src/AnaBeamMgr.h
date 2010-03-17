//#  AnaBeamMgr.h: interface of the AnaBeamMgr class
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

#ifndef ANABEAMMGR_H_
#define ANABEAMMGR_H_

#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Pointing.h>

#include <blitz/array.h>

#include "J2000Converter.h"
#include "AnalogueBeam.h"

namespace LOFAR {
  namespace BS {


class AnaBeamMgr {
public:
	// Constructor
	// @param nrRCUs  The number of rcus that are in one ring.
	// @param nrRings The number of active rings (1 | 2)
	AnaBeamMgr(uint		nrRCUS, uint		nrRings);

	// Destructor.
	~AnaBeamMgr();

	// Add a beam to the manager
	bool addBeam(const AnalogueBeam& beam);

	// Delete a beam to the manager
	void deleteBeam(const string& beamname);

	// Add a pointing to the administration
	bool addPointing(const string& beamName, const IBS_Protocol::Pointing& newPt);

	// Activate all possible analogue beams.
	// Note: activateBeams throws away pointings that are expired at the given timestamp. Calling this 
	//       routine with times that are in reverse order might give unexpected results.
	void activateBeams(const RTC::Timestamp&	now);

	// @return Current Direction in angle2Pi and anglePi.
	bool currentDirection(double&	angle2Pi, double&	anglePi);

	// Calculate delays for all 16 elements of all HBA tiles
	void calculateHBAdelays(RTC::Timestamp	targetTime, J2000Converter&	aJ2000Conv);

	// send the just calculated delays
	void sendHBAdelays(GCF::TM::GCFPortInterface&	port);

	// for easy debugging
	void showAdmin() const;

private:
	// Don't allow copying this object.
	AnaBeamMgr (const AnaBeamMgr&);            // not implemented
	AnaBeamMgr& operator= (const AnaBeamMgr&); // not implemented

	// Read the file containing the delta xyz values of the tile elements
	void getAllHBADeltas(const string& filename);
	
	// Read the file containing the delaysteps we can apply to the HBA signal path
	void getAllHBAElementDelays(const string& filename);
		
	class PointingInfo {
	public:
		PointingInfo() : active(false) {};
		~PointingInfo() {};
		AnalogueBeam			beam;
		IBS_Protocol::Pointing	pointing;
		bool					active;
		bool operator<	(const PointingInfo& that) {
			if (beam.rankNr() != that.beam.rankNr())    return (beam.rankNr() < that.beam.rankNr());
			if (active != that.active) 					return (active);
			if (beam.name() != that.beam.name())	 	return (beam.name() < that.beam.name()); 
			return (beam.currentPointing().time() < that.beam.currentPointing().time());
		};
	};
	
	//# ----- DATAMEMBERS -----
	// Constants
	uint	itsRCUsPerRing;
	uint	itsNrRings;
	double	itsMeanElementDelay;

	// Relative positions of the elements of the HBA tile.
	blitz::Array<double, 2>		itsTileRelPos;	// [N_HBA_ELEMENTS,x|y|z dipole] = [16,3]
	// Delay steps [0..31] of an element
	blitz::Array<double, 1>		itsDelaySteps;	// [N_HBA_DELAYS] = [32] 	

	// Processing information

	// The caluclated delays for each element for each HBA tile.
	blitz::Array<uint8, 2> 		itsHBAdelays;	// [rcus, N_HBA_ELEM_PER_TILE]

	// Time the delays were calculated for.
	RTC::Timestamp				itsTargetTime;

	// RCUs participating in the active beams
	bitset<MAX_RCUS>			itsActiveRCUs;

	// queue of future pointings as delivered by the user.
	map<string, AnalogueBeam>	itsBeams;
	list<PointingInfo>			itsPointings;
};

//# -------------------- inline functions --------------------

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* ANABEAMMGR_H_ */
