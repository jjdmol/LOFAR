//#  BlAllocArr.h: Tripled that describes an allocation of a beamlet
//#
//#  Copyright (C) 2009
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

#ifndef LOFAR_MD_PROTOCOL_BEAMLET_ALLOCATION_H
#define LOFAR_MD_PROTOCOL_BEAMLET_ALLOCATION_H

// \file
// Allocation tripled for 1 beamlet

//# Includes

namespace LOFAR {
  namespace MAC {

class BlAllocArr
{ 
public:
	// Allocation
	explicit BlAllocArr(int	nrElements);
	~BlAllocArr();

	BeamletAllocation&	operator[](uint	index);
	const BeamletAllocation&	operator[](uint	index) const;

private:
	// default construction not allowed.
	BlAllocArr();
	// copying is not allowed
	BlAllocArr(const BlAllocArr&	that);
	BlAllocArr& operator=(const BlAllocArr& that);

	// ----- Datamembers -----
	uint						itsSize;
	vector<BeamletAllocation>	itsPool;
} 

  } // namespace MAC
} // namespace LOFAR

#endif
