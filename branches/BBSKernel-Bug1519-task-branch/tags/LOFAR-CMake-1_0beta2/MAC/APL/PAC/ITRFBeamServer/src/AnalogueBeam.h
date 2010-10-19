//#  AnalogueBeam.h: interface of the analogueBeam class
//#
//#  Copyright (C) 2010
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

#ifndef ANALOGUE_BEAM_H_
#define ANALOGUE_BEAM_H_

#include <lofar_config.h>
#include "Beam.h"
#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

class AnalogueBeam : public Beam {
public:

	// Default constructor
	// @param name String identifying this beam uniquely in the OTDB, used with
	// key-value logger as nodeid.
	// @param rcuMask The RCUs that participate in this beam.
	// @param ringNr The serdes segment the allocation is ment for.
	AnalogueBeam(const string& 								name, 
				 const bitset<MAX_RCUS>&					rcuMask,
				 uint 										rankNr);
	AnalogueBeam() : itsRankNr(0) {};

	// Default destructor.
	virtual ~AnalogueBeam();

	// Get rank of this Beam
	uint rankNr() const	{ return (itsRankNr); }

	void calculateHBAdelays(RTC::Timestamp					timestamp,
						    const blitz::Array<double,2>&	tileDeltas,
						    const blitz::Array<double,1>&	elementDelays);
private:
	// Don't allow copying this object.
//	AnalogueBeam (const AnalogueBeam&);            // not implemented
//	AnalogueBeam& operator= (const AnalogueBeam&); // not implemented

	//# ----- DATAMEMBERS -----

	// the ranknumber of the beam (lower is more important)
	int		itsRankNr;
};

//# -------------------- inline functions --------------------

  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* ANALOGUEBEAM_H_ */
