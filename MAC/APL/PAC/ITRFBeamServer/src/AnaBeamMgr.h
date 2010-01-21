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
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Pointing.h>

#include <queue>
#include <blitz/array.h>

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
	void deleteBeam(const AnalogueBeam& beam);

	// Activate all possible analogue beams
	void activateBeams(const RTC::Timestamp&	now);

	// @return Current Direction in angle2Pi and anglePi.
	bool currentDirection(double&	angle2Pi, double&	anglePi);

	void calculateHBAdelays(RTC::Timestamp					timestamp,
						    const blitz::Array<double,2>&	tileDeltas,
						    const blitz::Array<double,1>&	elementDelays);

	// for easy debugging
	void showAdmin() const;

private:
	// Don't allow copying this object.
	AnaBeamMgr (const AnaBeamMgr&);            // not implemented
	AnaBeamMgr& operator= (const AnaBeamMgr&); // not implemented

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
	uint				itsRCUsPerRing;
	uint				itsNrRings;

	// RCUs participating in the active beams
	bitset<MAX_RCUS>	itsActiveRCUs;

	// queue of future pointings as delivered by the user.
	map<string, AnalogueBeam>	itsBeams;
	list<PointingInfo>			itsPointings;
};

//# -------------------- inline functions --------------------

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* ANABEAMMGR_H_ */
