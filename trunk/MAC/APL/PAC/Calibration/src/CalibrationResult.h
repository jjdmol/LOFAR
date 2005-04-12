//#  -*- mode: c++ -*-
//#  CalibrationResult.h: class definition of the CalibrationResult class which reprsentents
//#               the calibration of a subarray
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

#ifndef CALIBRATIONRESULT_H_
#define CALIBRATIONRESULT_H_

#include <math.h>
#include <blitz/array.h>

namespace CAL
{
  class CalibrationResult
    {
    public:
      CalibrationResult(int nantennas, int nsubbands);
      virtual ~CalibrationResult();

      /**
       * Get reference to the result gains array.
       */
      const blitz::Array<std::complex<double>, 3>& getGains() const { return m_gains; }

      /**
       * Get reference to the quality array.
       */
      const blitz::Array<double, 1>& getQuality() const { return m_quality; }

      /**
       * has the calibration algorithm producing this result completed?
       */
      bool isComplete() { return m_complete; }

      /**
       * set the complete status.
       */
      void setComplete(bool value) { m_complete = value; }

    private:
      blitz::Array<std::complex<double>, 3> m_gains;
      blitz::Array<double, 1>               m_quality;
      bool                                  m_complete; // is this calibration complete?
    };
};

#endif /* CALIBRATIONRESULT_H_ */

