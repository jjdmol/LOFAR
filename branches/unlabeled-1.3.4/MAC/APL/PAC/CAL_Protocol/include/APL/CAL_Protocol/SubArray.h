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

#include <set>
#include <string>

#include "Subject.h"

#include "SpectralWindow.h"
#include "AntennaArray.h"
#include "ACC.h"
#include "AntennaGains.h"

namespace LOFAR {
  namespace CAL {

    class CalibrationInterface; // forward declaration

    class SubArray : public AntennaArray, public RTC::Subject
    {
    public:

      /**
       * Construct a subarray.
       * @param name   The name of the subarray.
       * @param array  The array of which this is a subarray.
       * @param select Select for each polarization dipole of each antenna whether it is included (true) in the subarray.
       * @param spw    The spectral window of this subarray.
       */
      SubArray(std::string                    name,
	       const blitz::Array<double, 3>& array,
	       const blitz::Array<bool, 2>&   select,
	       double sampling_frequency,
	       int spectral_window,
	       int nsubbands);
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
      bool getGains(const AntennaGains*& cal, int buffer = FRONT);

      /**
       * Abort background calibration.
       */
      void abortCalibration();

      /**
       * Check whether calibration has completed.
       */
      bool isDone();

      /**
       * Get a reference to the spectral window for this subarray.
       */
      const SpectralWindow& getSPW() const;

      /**
       * Enumeration of buffer positions.
       */
      enum {
	FRONT = 0,
	BACK = 1
      };

    private:
      const SpectralWindow m_spw;              // reference to the spectral window for this subarray
      AntennaGains*        m_result[BACK + 1]; // two calibration result records
    };

    class SubArrays
    {
    public:
      SubArrays();
      virtual ~SubArrays();

      /**
       * Add new subarray to the subarrays.
       * @param array Pointer (not 0) to the array to be added.
       */
      void add(SubArray* array);

      /**
       * Remove subarray with the given name from the subarrays.
       * The subarray instance will be deleted.
       * @param name Name of the subarray to remove.
       * @return true if found and removed, false otherwise
       */
      bool remove(std::string name);
      bool remove(SubArray*& subarray);

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

    private:
      std::map<std::string, SubArray*> m_arrays;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SUBARRAY_H_ */

