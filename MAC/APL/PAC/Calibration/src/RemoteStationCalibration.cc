//#  -*- mode: c++ -*-
//#  RemoteStationCalibration.cc: class implementation of RemoteStationCalibration
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

#include "RemoteStationCalibration.h"

#include <blitz/array.h>

using namespace CAL;
using namespace blitz;

void RemoteStationCalibration::calibrate(const SubArray& subarray, CalibrationResult& result)
{
  /**
   * Parameter access:
   *
   * this->getACC():           get reference to the ACC matrix
   * this->getDipoleModel():   get const reference to the dipole sensitivity/gain model
   * this->getSourceCatalog(): get const reference to the sky model
   *
   * subarray.getSPW():        get const reference to the spectral window
   * subarray.getAntennaPos(): get const reference to the array with dipole positions
   */

  const SpectralWindow&  spw = subarray.getSPW();
  const Array<double, 3> pos = subarray.getAntennaPos();

  // when finished
  result.setComplete(true);
}


