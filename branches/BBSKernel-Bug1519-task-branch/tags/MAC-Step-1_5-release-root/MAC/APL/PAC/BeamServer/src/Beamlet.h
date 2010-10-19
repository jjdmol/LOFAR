//#  Beamlet.h: interface of the Beamlet class
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

#ifndef BEAMLET_H_
#define BEAMLET_H_

#include <APL/CAL_Protocol/SpectralWindow.h>
#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

//# forward declarations
class Beam;

class Beamlet
{
public:
	// Constructor
	Beamlet();

	// Destructor
	virtual ~Beamlet();

	// Allocate the beamlet.
	// @param subband Index of the subband to allocate
	// @param nsubbands Maximum number of subbands
	// within the spectral window.
	// @return 0 if allocation succeeded, < 0 otherwise.
	int allocate(const Beam& beam, int subband, int nsubbands);

	// Deallocate the beamlet
	int deallocate();

	// If the beam is allocated return true,
	// otherwise return false.
	bool allocated() const;

	// Get pointer to spectral window for this beamlet.
	const CAL::SpectralWindow& getSPW() const;

	// Get index (from 0) of the subband within the spectral window.
	int subband() const;

	// Get absolute index of this beamlet in the array
	// of all beamlets.
	int index() const;

	// Get beam.
	const Beam* getBeam() const;

private:
	// Don't allow copying this object.
	Beamlet (const Beamlet&); // not implemented
	Beamlet& operator= (const Beamlet&); // not implemented

	//# --- datamembers ---

	// subband within the spectral window 
	int m_subband;

	// Index of the beam to which this beamlet belongs.
	// -1 means beamlet is not allocated.
	// >= 0 means beamlet is allocated to the beam
	// with the specified index.
	const Beam* m_beam;
};

inline bool	Beamlet::allocated() const	{ return m_beam != 0; }
inline int	Beamlet::subband()   const	{ return m_subband; }

  }; // namespace BS
}; // namespace LOFAR

#endif /* BEAMLET_H_ */
