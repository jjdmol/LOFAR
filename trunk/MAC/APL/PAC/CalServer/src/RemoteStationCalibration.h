//#  -*- mode: c++ -*-
//#  RemoteStationCalibration.h: class definition for the Beam Server task.
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

#ifndef REMOTESTATIONCALIBRATION_H_
#define REMOTESTATIONCALIBRATION_H_

#include "SourceCatalog.h"
#include "DipoleModel.h"
#include "CalibrationAlgorithm.h"
#include "CalibrationResult.h"
#include "SubArray.h"
#include <blitz/array.h>

namespace CAL
{
  class RemoteStationCalibration : public CalibrationAlgorithm
    {
    public:
      RemoteStationCalibration(const SourceCatalog& catalog, const DipoleModel& dipolemodel);

      /**
       * Destructor: delete any dynamically allocated member variables
       */
      virtual ~RemoteStationCalibration() {}

      virtual void calibrate(const SubArray& subarray, const ACC& acc, CalibrationResult& result);
      
    private:
      const std::vector<Source> make_local_sky_model(const SourceCatalog& catalog, double obstime);

      blitz::Array<std::complex<double>, 2> make_ref_acm(const std::vector<Source>& LSM, blitz::Array<double, 3>& AntennaPos, const DipoleModel& dipolemodel, double freq);
      blitz::Array<bool, 2> set_restriction(blitz::Array<double, 3>& AntennaPos, double minbaseline);
      blitz::Array<std::complex<double>, 2> computeAlpha(blitz::Array<std::complex<double>, 2>& acm, blitz::Array<std::complex<double>, 2>& R0, blitz::Array<bool, 2> restriction);

      blitz::Array<double, 2> matmult(blitz::Array<double, 2> A, blitz::Array<double, 2> B);
      double interp1d(blitz::Array<double, 1> xval, blitz::Array<double, 1> yval, double xinterp);
      double interp2d(blitz::Array<double, 1> xgrid, blitz::Array<double, 1> ygrid, blitz::Array<double, 2> dataval, double xinterp, double yinterp);
      double interp3d(blitz::Array<double, 1> xgrid, blitz::Array<double, 1> ygrid, blitz::Array<double, 1> zgrid, blitz::Array<double, 3> dataval, double xinterp, double yinterp, double zinterp);

      // member variables needed to store local state
    };
};

#endif /* REMOTESTATIONCALIBRATION_H_ */

