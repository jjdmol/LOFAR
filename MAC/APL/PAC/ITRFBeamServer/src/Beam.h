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
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/LofarConstants.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Pointing.h>

namespace LOFAR {
  namespace BS {

// Class representing a single beam allocated by a client
// using a BEAMALLOC event.
class Beam {
public:

	// Default constructor
	// @param name String identifying this beam uniquely in the OTDB, used with
	// key-value logger as nodeid.
	// @param rcuMask The RCUs that participate in this beam.
	Beam(const string& 				name, 
		 const bitset<MAX_RCUS>&	rcuMask);
	Beam() {};

	// Default destructor.
	virtual ~Beam();

	// Add a pointing to a beam if it not overlaps with other pointings
	bool addPointing(const IBS_Protocol::Pointing& pointing);

	// @return Current pointing.
	IBS_Protocol::Pointing currentPointing() const { return (itsCurrentPointing); } 

	// @return Current pointing.
	IBS_Protocol::Pointing pointingAtTime(const RTC::Timestamp& time);

	// Get all pointings in a vector. They are sorted in time order.
	vector<IBS_Protocol::Pointing> getAllPointings() const;

	// Get beam name (unique name, can be used as key in key-value logger).
	string name() const { return (itsName); }

	// Get beam name (unique name, can be used as key in key-value logger).
	const bitset<MAX_RCUS>& rcuMask() const { return (itsRCUs); }

	// Output functions for easy debugging
	void showPointings () const;

	// print function for operator<<
	ostream& print (ostream& os) const;

private:
	// Don't allow copying this object.
//	Beam (const Beam&);            // not implemented
//	Beam& operator= (const Beam&); // not implemented

	// Method to undo an allocation.
	void deallocate();

	void _resolveGaps();
	bool _pointingOverlaps(const IBS_Protocol::Pointing& pt) const;

	//# ----- DATAMEMBERS -----
	// Name used as key in key-value logger.
	string 		itsName;

	// RCUs participating in the beam
	bitset<MAX_RCUS>		itsRCUs;

	// current direction of the beam
	IBS_Protocol::Pointing 	itsCurrentPointing;

	// queue of future pointings as delivered by the user.
	list<IBS_Protocol::Pointing>	itsPointings;
};

//# -------------------- inline functions --------------------
//
// operator <<
//
inline ostream& operator<<(ostream& os, const Beam& beam)
{
	return (beam.print(os));
}


  }; //# namepsace BS
}; //# namespace LOFAR

#endif /* BEAM_H_ */
