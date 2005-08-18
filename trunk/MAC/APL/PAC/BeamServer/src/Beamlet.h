//#  ABSBeamlet.h: interface of the Beamlet class
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

#include <SpectralWindow.h>
#include <blitz/array.h>

// define the datatype for the weight calculation
#define W_TYPE_DOUBLE
#ifdef W_TYPE_DOUBLE
#define W_TYPE double
#define W_TYPE_DOUBLE
#else
#define W_TYPE float
#define W_TYPE_FLOAT
#endif

namespace LOFAR {
  namespace BS {

    class Beam;

    class Beamlet
      {
      public:
	/**
	 * Constructor
	 */
	Beamlet();

	/**
	 * Destructor
	 */
	virtual ~Beamlet();

	/**
	 * Allocate the beamlet.
	 * @param subband Index of the subband to allocate
	 * @param nsubbands Maximum number of subbands
	 * within the spectral window.
	 * @return 0 if allocation succeeded, < 0 otherwise.
	 */
	int allocate(const Beam& beam, int subband, int nsubbands);

	/**
	 * Deallocate the beamlet
	 */
	int deallocate();

	/**
	 * If the beam is allocated return true,
	 * otherwise return false.
	 */
	bool allocated() const;

	/**
	 * Get pointer to spectral window for this beamlet.
	 */
	const CAL::SpectralWindow* getSPW() const;

	/**
	 * Get index (from 0) of the subband within the spectral window.
	 */
	int subband() const;

	/**
	 * Get absolute index of this beamlet in the array
	 * of all beamlets.
	 */
	int index() const;

	/**
	 * Get beam.
	 */
	const Beam* getBeam() const;


      private:
	/** subband within the spectral window */
	int m_subband;

	// /** index of the beamlet in the total beamles array */
	// int m_index;

	/**
	 * Index of the beam to which this beamlet belongs.
	 * -1 means beamlet is not allocated.
	 * >= 0 means beamlet is allocated to the beam
	 * with the specified index.
	 */
	const Beam* m_beam;

      private:
	/**
	 * Don't allow copying this object.
	 */
	Beamlet (const Beamlet&); // not implemented
	Beamlet& operator= (const Beamlet&); // not implemented
      };

    inline bool  Beamlet::allocated() const                   { return m_beam != 0; }
    inline int   Beamlet::subband()   const                   { return m_subband; }
    //inline int  Beamlet::index()     const            { return m_index; }

    /**
     * Factory class for beamlets. It manages a fixed set of beamlet
     * instances.
     */
    class Beamlets {

    public:

      /**
       * Constructor
       */
      Beamlets(int nbeamlets);

      /**
       * Destructor
       */
      virtual ~Beamlets();

      /**
       * return beamlet at specified index
       * @param index
       * @return Beamlet& the specified beamlet
       */
      Beamlet* get(int index) const;
      
      /**
       * Calculate weights for all beamlets 
       * for the specified number of time steps.
       */
      void calculate_weights(const blitz::Array<W_TYPE, 3>&         pos,
			     blitz::Array<std::complex<W_TYPE>, 3>& weights);

    private:
      // default constructor not allowed
      Beamlets(); // no implementation

    private:
      Beamlet* m_beamlets;
      int      m_nbeamlets;
    };
  };
};
     
#endif /* BEAMLET_H_ */
