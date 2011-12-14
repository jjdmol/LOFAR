//# LofarATerm.h: Compute the LOFAR beam response on the sky.
//#
//# Copyright (C) 2011
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
//# $Id: LOFARATerm.h 18046 2011-05-19 20:58:40Z diepen $

#ifndef LOFAR_LOFARFT_LOFARATERM_H
#define LOFAR_LOFARFT_LOFARATERM_H

#include <Common/LofarTypes.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

namespace casa
{
  class DirectionCoordinate;
  class MEpoch;
  class MeasurementSet;
  class Path;
}

namespace LOFAR
{

  class LofarATerm
  {
  public:
    LofarATerm();

    virtual vector<casa::Cube<casa::Complex> > evaluate(const casa::IPosition &shape,
      const casa::DirectionCoordinate &coordinates,
      uint station,
      const casa::Vector<casa::Double> &freq,
      bool normalize = false) = 0;
    virtual double resolution() = 0;
    virtual void setEpoch( const casa::MEpoch &epoch )=0;
  

  };
} // namespace LOFAR

#endif
