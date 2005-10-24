//#  -*- mode: c++ -*-
//#  SubArray.h: class definition for the SubArray class
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

#ifndef SUBARRAY_H_
#define SUBARRAY_H_

#include <map>
#include <list>
#include <string>

#include <APL/RTCCommon/Subject.h>

#include "SpectralWindow.h"
#include "AntennaArray.h"
/*#include "ACC.h"*/
#include "SharedResource.h"
#include "AntennaGains.h"

namespace LOFAR {
  namespace CAL {

    // forward declarations
    class ACC;
    class CalibrationInterface;

    class SubArray : public AntennaArray, public RTC::Subject
    {
    public:

      /**
       * Default constructor.
       */
      SubArray();

      /**
       * Construct a subarray.
       * @param name   The name of the subarray.
       * @param geoloc The geographical location of the subarray.
       * @param pos    The antenna positions of the parent array elements (nantennas x npolarizations x 3-coordinates).
       * @param select Select for each polarization dipole of each antenna whether it is included (true) in the subarray.
       * @param sampling_frequency The sampling frequency this runs at.
       * @param nyquist_zone The nyquist zone in which we wish to measure.
       * @param nsubbands The number of subbands of the spectral window.
       * @param rcucontrol The RCU control setting (LB, HBL, HBH, etc).
       */
      SubArray(std::string                    name,
	       const blitz::Array<double, 1>& geoloc,
	       const blitz::Array<double, 3>& pos,
	       const blitz::Array<bool, 2>&   select,
	       double                         sampling_frequency,
	       int                            nyquist_zone,
	       int                            nsubbands,
	       uint8                          rcucontrol);
      virtual ~SubArray();

      /**
       * Start (background) calibration of the subarray
       * using the specified algorithm and ACC as input.
       * @param cal The calibration algorithm to use.
       * @param acc The Array Correlation Cube on which to calibrate.
       */
      void calibrate(CalibrationInterface* cal, const ACC& acc);

      /**
       * Get calibration result (if available).
       * @param cal Calibration result
       */
      bool getGains(AntennaGains*& cal, int buffer = FRONT);

      /**
       * Abort background calibration.
       */
      void abortCalibration();

      /**
       * Check whether calibration has completed.
       */
      bool isDone();

      /**
       * Used to clear the 'done' flag after updating all subscriptions.
       */
      void clearDone();

      /**
       * Get a reference to the spectral window for this subarray.
       */
      const SpectralWindow& getSPW() const;

      /**
       * Assignement operator.
       */
      SubArray& operator=(const SubArray& rhs);

      /**
       * Enumeration of buffer positions.
       */
      enum {
	FRONT = 0,
	BACK = 1
      };

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack   (void* buffer);
      unsigned int unpack (void* buffer);
      /*@}*/

    private:

      /* prevent copy */
      SubArray(const SubArray& other); // no implementation

      SpectralWindow m_spw;              // the spectral window for this subarray
      AntennaGains*  m_result[BACK + 1]; // two calibration result records
    };

    class SubArrays : public SharedResource
    {
    public:
      SubArrays();
      virtual ~SubArrays();

      /**
       * @pre array != 0
       * Schedule a new array for addition to the subarray list.
       * @param array Pointer (not 0) to the array to be added.
       */
      void schedule_add(SubArray* array);

      /**
       * Schedule subarray for removal. This is used
       * to keep a subarray while the calibration algorithm
       * is running on a separate thread. Only really remove
       * the subarray when the thread has finished.
       */
      bool schedule_remove(std::string name);
      bool schedule_remove(SubArray*& subarray);

      /**
       * New subarrays are listed in m_new_arrays.
       * This method moves these subarrays to the m_arrays map.
       */
      void creator();
      
      /**
       * Subarrays that should be removed are listed in m_dead_arrays.
       * This method deletes these subarrays removes
       * them from the m_arrays.
       */
      void undertaker();

      /**
       * Find a subarray by name.
       * @param name Find the subarray with this name.
       * @return pointer to the subarray if found, 0 otherwise.
       */
      SubArray* getByName(std::string name);

      /**
       * This is called periodically to check whether any of the subarrays
       * have completed calibration and need to inform their subscribers of
       * the new calibration weights.
       */
      void updateAll();

      /**
       * Try to start calibration on all subarrays with the give
       * algorithm and array correlation cube.
       * @param cal Pointer to the calibration algorithm interface.
       * @param acc Reference to the array correlation cube to use for
       * calibration.
       */
      void calibrate(CalibrationInterface* cal, ACC& acc);

      /**
       * Remove subarray with the given name from the subarrays.
       * The subarray instance will be deleted.
       * @param name Name of the subarray to remove.
       * @return true if found and removed, false otherwise
       */
    private:
      bool remove(std::string name);
      bool remove(SubArray*& subarray);

    private:
      std::map<std::string, SubArray*> m_new_arrays;  // subarrays that should be added for the next run of the calibration algorithm
      std::map<std::string, SubArray*> m_arrays;
      std::list<SubArray*>             m_dead_arrays; // subarrays that have been stopped
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SUBARRAY_H_ */

