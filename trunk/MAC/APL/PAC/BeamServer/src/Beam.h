//#  ABSBeam.h: interface of the Beam class
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

#ifndef BEAM_H_
#define BEAM_H_

#include "Pointing.h"
#include "Beamlet.h"
#include "Beamlet2SubbandMap.h"
#include <Timestamp.h>
#include <SpectralWindow.h>
#include <time.h>

#include <queue>
#include <set>
#include <list>

#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

    /**
     * Class representing a single beam allocated by a client
     * using a BEAMALLOC event.
     */
    class Beam {
    public:

      /**
       * Default constructor
       */
      Beam(double sampling_frequency, int nyquist_zone, int nsubbands);

	
      /**
       * Default destructor.
       */
      virtual ~Beam();

      /**
       * Allocate a new beam.
       * @param allocation Allocation of beamlet (indices) to subband (indices)
       * @param beamlets Allocate from this set of beamlets.
       * @return bool true if allocation successful, false if allocation failed
       * because specified beamlet has already been allocated or there is 
       * a range problem with the beamlet or subband indices.
       */
      bool allocate(BS_Protocol::Beamlet2SubbandMap allocation, Beamlets& beamlets);

      /**
       * Modify beam by chaning the subbands
       * of the beamlet2subband mapping. The lhs of the mapping
       * can not be changed.
       * @return bool true if modification succeeded, false if it failed
       * because beamlet set was changed or if there is a range problem
       * with the beamlet or subband indices.
       */
      bool modify(BS_Protocol::Beamlet2SubbandMap allocation);

      /**
       * Get the allocation mapping for this beam.
       * @return Beamlet2SubbandMap the mapping from beamlet to subband for this beam.
       */
      BS_Protocol::Beamlet2SubbandMap getAllocation() const;

      /**
       * @return Current pointing.
       */
      inline Pointing getPointing() const { return m_pointing; }

      /**
       * Add a pointing to a beam.
       */
      void addPointing(const Pointing& pointing);

      /**
       * Convert coordinates from the m_pointing_queue
       * to the local coordinate system, for times >= begintime
       * and begintime < begintime + m_compute_interval.
       * Converted coordinates are put on the m_coordinate_track
       * queue.
       * @param begintime First time of pointing to convert, this is typically
       * the last time the method was called. E.g.
       * @code
       * begintime=lasttime;
       * gettimeofday(&lasttime, 0);
       * lasttime.tv_sec += compute_interval; // compute_interval seconds ahead in time
       * for (beam in beams)
       * {
       *   // convert coordinate for next compute_interval seconds
       *   beam->convertPointings(begintime, compute_interval);
       * }
       * @endcode
       * @param compute_interval the interval for which pointings must be computed
       * @return int 0 if successful, < 0 otherwise
       */
      int convertPointings(RTC::Timestamp begintime, int compute_interval);

      /**
       * Get converted time-stamped coordinates frobm the queue.
       * This method is called by the Beamlet class to get a priority
       * queue of coordinates.
       * @return array with the coordinates for the next period.
       */
      const blitz::Array<W_TYPE,2>& getLMNCoordinates() const;

    private: // methods

      /**
       * Method to undo an allocation.
       */
      void deallocate();

    private:

      /**
       * Allocation.
       */
      BS_Protocol::Beamlet2SubbandMap m_allocation;

      /**
       * SpectralWindow
       */
      CAL::SpectralWindow m_spw;
	  
      /** current direction of the beam */
      Pointing m_pointing;

      /** queue of future pointings */
      std::priority_queue<Pointing> m_pointing_queue;

      /**
       * Current coordinate track in station local coordinates.
       * Two dimensional array for (l,m,n) coordinates
       * The first dimension is always 3 (for the three
       * coordinates) the second dimension is m_compute_interval
       */
      blitz::Array<W_TYPE,2> m_azels; // az,el coordinates
      blitz::Array<W_TYPE,2> m_lmns;  // l,m,n coordinates

      /**
       * Set of beamlets belonging to this beam.
       * It is a set because there should be no
       * duplicate beamlet instances in the set.
       */
      std::set<Beamlet*> m_beamlets;

    private:
      /**
       * Don't allow copying this object.
       */
      Beam (const Beam&);            // not implemented
      Beam& operator= (const Beam&); // not implemented

    private:
      /**
       * Private constants.
       */
      static const int N_TIMESTEPS = 20; // number of timesteps to calculate ahead
    };

    /**
     * Factory class for Beam. This class manages the collection of Beams
     * that are active in the BeamServer at a particular point in time.
     */
    class Beams {
  
    public:
      Beams(int nbeamlets);

      /**
       * Create a new beam.
       */
      Beam* get(BS_Protocol::Beamlet2SubbandMap allocation,
		double sampling_frequency, int nyquist_zone);

      /**
       * Check if a beam exists.
       */
      bool exists(Beam* beam);

      /**
       * Destroy a beam. The beam is 'delete'ed.
       * @return bool true if beam has been found and deleted, false otherwise
       */
      bool destroy(Beam* beam);

      /**
       * Calculate weights for all beamlet of all beams
       * for the specified number of time steps.
       */
      void calculate_weights(RTC::Timestamp timestamp,
			     int compute_interval,
			     const blitz::Array<W_TYPE, 3>&         pos,
			     blitz::Array<std::complex<W_TYPE>, 3>& weights);


      /**
       * Return the combined beamlet to subband
       * mapping for all beams.
       */
      BS_Protocol::Beamlet2SubbandMap getSubbandSelection();

    private:
      /**
       * List of active beams.
       */
      std::list<Beam*> m_beams;
      Beamlets         m_beamlets; // collection of all beamlets
    };

  };
};
     
#endif /* BEAM_H_ */
