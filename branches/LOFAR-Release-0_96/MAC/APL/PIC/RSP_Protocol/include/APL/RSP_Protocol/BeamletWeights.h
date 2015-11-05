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
      /**
       * Constants.
       */
      static const int SINGLE_TIMESTEP = 1;

      /**
       * Constructors for a BeamletWeights object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      BeamletWeights() { }
	  
      /* Destructor for BeamletWeights. */
      virtual ~BeamletWeights() {}

      static const int NDIM = 3; // dimension (N_POL) REMOVED, now using rcumask

      /* get reference to the weights array */
      blitz::Array<std::complex<int16>, NDIM>& operator()();

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      /**
       * The beamlet weights.
       * Dimension 1: nr_timesteps (>1)
       * Dimension 2: count(rcumask)
       * Dimension 3: N_BEAMLETS
       * REMOVED Dimension 4, now using rcumask...
       */
      blitz::Array<std::complex<int16>, NDIM> m_weights;
    };

    inline blitz::Array<std::complex<int16>, RSP_Protocol::BeamletWeights::NDIM>& BeamletWeights::operator()() { return m_weights; }
  };
}; //namespace LOFAR
#endif /* BEAMLETWEIGHTS_H_ */
