//#  -*- mode: c++ -*-
//#
//#  BeamletWeights.h: beamlet weights class
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

#ifndef BEAMLETWEIGHTS_H_
#define BEAMLETWEIGHTS_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

class BeamletWeights
{
public:
	// Constants.
	static const int SINGLE_TIMESTEP = 1;

	// The ebeamlet concept is used to be able to address the Wx and Wy weights 
	// for each RCU/subband signal independently.
	// Therefore, we use the following view:
	// One LOFAR beamlet consist of two ebeamlets. The first ebeamlet is associated 
	// with the Wx weights and the second ebeamlet is associated with the Wy weights.
	// 
	// The dimension of the itsWeights array is increased to 4 and we use the third
	// dimension to select between X or Y weights. 
	// 
	// This behaviour can be selected with the --weightselect option of the --weights 
	// command.
	static const int N_EBEAMLETS     = 2; // number of ebeamlets in one LOFAR beamlet

	static const int NDIM = 4; // increased for e-beamlet mgt.

	static const int SELECT_X_IS_Y  = -1;
	static const int SELECT_X_ONLY  =  0;
	static const int SELECT_Y_ONLY  =  1;
	static const int SELECT_X_AND_Y =  2;

	// Constructors for a BeamletWeights object.
	BeamletWeights() : itsWeightSelect(-1) { }

	// Destructor for BeamletWeights.
	virtual ~BeamletWeights() {}

	// get reference to the weights array
	blitz::Array<std::complex<int16>, NDIM>& operator()()
		{ return (itsWeights); }

	// Accessors to the weightSelector
	// Note: 0:X, 1:Y, 2:X+Y delivered in weights-array.
	bool weightSelect(int	aWeightSelect);
	int	 weightSelect() const { return (itsWeightSelect); }

	/*@{*/
	// marshalling methods
	unsigned int getSize();
	unsigned int pack  (void* buffer);
	unsigned int unpack(void *buffer);
	/*@}*/

private:
	// The beamlet weights.
	// Dimension 1: nr_timesteps (>1)
	// Dimension 2: count(rcumask)
	// Dimension 3: Select between X or Y weights
	// Dimension 4: N_BEAMLETS
	blitz::Array<std::complex<int16>, NDIM> itsWeights;
	// WeightSelect
	// 0:only X, 1:only Y, 2: both are specified
	// Note: weightSelect value must match dimension of the itsWeightsArray ofcourse.
	int32			itsWeightSelect;
};

  }; // namespace RSP_Protocol
}; //namespace LOFAR

#endif /* BEAMLETWEIGHTS_H_ */
