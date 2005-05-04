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

#include "SpectralWindow.h"
#include "AntennaArray.h"
#include "ACC.h"
#include "CalibrationResult.h"

namespace CAL
{
  class CalibrationInterface; // forward declaration

  class SubArray : public AntennaArray
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
	     const SpectralWindow&          spw);
    virtual ~SubArray();

    /**
     * Start (background) calibration of the subarray
     * using the specified algorithm and ACC as input.
     * @param cal The calibration algorithm to use.
     * @param acc The Array Correlation Cube on which to calibrate.
     */
    void startCalibration(CalibrationInterface* cal, const ACC& acc);

    /**
     * Get calibration result (if available).
     * @param cal Calibration result
     */
    bool getCalibration(const CalibrationResult*& cal, int buffer = FRONT);

    /**
     * Abort background calibration.
     */
    void abortCalibration();

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
    const SpectralWindow& m_spw; // reference to the spectral window for this subarray
    CalibrationResult*    m_result[BACK + 1]; // two calibration result records
  };
};

#endif /* SUBARRAY_H_ */

