//# util.cc:
//#
//# Copyright (C) 2014
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: $

#include <lofar_config.h>

#include <LofarFT/util.h>

#include <Common/LofarLogger.h>

#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

namespace LOFAR {
namespace LofarFT {  

casa::Double get_reference_frequency(ParameterSet parset, casa::MeasurementSet ms)
{
  casa::Double reffreq = parset.getDouble("image.reffreq", 0.0);
  
  if (reffreq == 0.0)
  {
    casa::uInt idDataDescription = 0;
    casa::ROMSDataDescColumns desc(ms.dataDescription());
    ASSERT(desc.nrow() > idDataDescription);
    ASSERT(!desc.flagRow()(idDataDescription));

    const casa::uInt idWindow = desc.spectralWindowId()(idDataDescription);

    // Get spectral information.
    casa::ROMSSpWindowColumns window(ms.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    reffreq = window.refFrequency()(idWindow);
  }
  return reffreq;
}

  
} // namespace LofarFT
} // namespace LOFAR