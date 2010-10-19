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

#ifndef ABSBEAM_H_
#define ABSBEAM_H_

#include "ABSPointing.h"
#include "ABSSpectralWindow.h"
#include "SpectralWindowConfig.h"
#include "ABSBeamlet.h"
#include <time.h>

#include <queue>
#include <set>
#include <map>

#include <blitz/array.h>

namespace ABS
{

  /**
   * Singleton class with exactly nbeams instances.
   */
  class Beam
      {
      public:
	  static const int TIMESTEP = 1; // in seconds
	  static const int N_TIMESTEPS = 20; // number of timesteps to calculate ahead

	  /**
	   * Get the beam with the matching handle.
	   * @return 0 on invalid handle. Beam must have
	   * been allocated before.
	   */
	  static Beam* getFromHandle(int handle);

	  /**
	   * Set the number of beam instances that
	   * should be created. After calling this method
	   * once you can not alter the number of 
	   * instances by calling it again.
	   * @param ninstances The total number of instances
	   * that should be created.
	   */
	  static int init(int ninstances,
			  unsigned short update_interval,
			  unsigned short compute_interval);

	  /**
	   * Allocate the beam using the specified subband set
	   * within the specified spectral window.
	   * @param spw Spectral window in which to allocate.
	   * @param subbands Which set of subbands to allocate
	   * (indices in the spectral window).
	   */
	  static Beam* allocate(int spw_index, std::set<int> subbands);

	  /**
	   * Return allocation status of a beam.
	   */
	  bool allocated() const;

	  /**
	   * @return Opaque handle of the beam.
	   */
	  int handle() const;

	  /**
	   * @return Current pointing.
	   */
	  Pointing pointing() const;

	  /**
	   * Deallocate a beam.
	   */
	  int deallocate();

	  /**
	   * Add a pointing to a beam.
	   */
	  int addPointing(const Pointing& pointing);

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
	   *   beam->convertPointings(begintime);
	   * }
	   * @endcode
	   * starting at time.
	   */
	  int convertPointings(time_t begintime);

	  /**
	   * Get converted time-stamped coordinates from the queue.
	   * This method is called by the Beamlet class to get a priority
	   * queue of coordinates.
	   * @return array with the coordinates for the next period.
	   */
	  const blitz::Array<W_TYPE,2>& getLMNCoordinates() const;

	  /**
	   * Get the mapping from input subbands to
	   * output beamlets.
	   * @note The map is NOT cleared.
	   * New pairs are simply added. This allows the
	   * caller to call this on a number of beams
	   * to get the total mapping for all beams.
	   */
	  void getSubbandSelection(std::map<int, int>& selection) const;

      protected:
	  Beam(); // no construction outside class
	  virtual ~Beam();

      private:
	  /** is this beam in use? */
	  bool m_allocated;
	  
	  /** current direction of the beam */
	  Pointing m_pointing;

	  /** index of this beam */
	  int m_index;

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
	   * Get the next available beam.
	   * @return 0 if there are no more beams available.
	   */
	  static Beam* getInstance();

	  //@{
	  /** singleton implementation members */
	  static int            m_ninstances;
	  static Beam*          m_beams; // array of ninstances beams
	  static unsigned short m_update_interval;
	  static unsigned short m_compute_interval;
	  //@}

      private:
	  /**
	   * Don't allow copying this object.
	   */
	  Beam (const Beam&); // not implemented
	  Beam& operator= (const Beam&); // not implemented
      };

  inline bool Beam::allocated() const { return m_allocated; }
  inline int  Beam::handle()    const { return m_index;     }
  inline Pointing Beam::pointing() const { return m_pointing; }
};
     
#endif /* ABSBEAM_H_ */
