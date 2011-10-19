//#  Beamlets.h: interface of the Beamlet class
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

#ifndef BEAMLETS_H_
#define BEAMLETS_H_

#include <APL/CAL_Protocol/SpectralWindow.h>
#include <blitz/array.h>
#include "Beamlet.h"

namespace LOFAR {
  namespace BS {

// Factory class for beamlets. It manages a fixed set of beamlet
// instances.
class Beamlets {
public:
	// Constructor
	Beamlets(int maxBeamletsEver);

	// Destructor
	virtual ~Beamlets();

	// return beamlet at specified index
	// @param index
	// @return Beamlet& the specified beamlet
	Beamlet* get(int index) const;

	// Calculate weights for all beamlets 
	// for the specified number of time steps.
	void calculate_weights(blitz::Array<std::complex<double>, 3>& weights);

private:
	// default constructor not allowed
	Beamlets(); // no implementation

	Beamlet*	itsBeamletsArray;
	int			itsMaxBeamlets;
};

  }; // namespace BS
}; // namespace LOFAR

#endif /* BEAMLETS_H_ */
