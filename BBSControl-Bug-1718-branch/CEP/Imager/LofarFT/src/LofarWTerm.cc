//# LofarWTerm.cc: Compute the LOFAR W-term
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
//# $Id$

#include <lofar_config.h>
#include <LofarFT/LofarWTerm.h>

using namespace casa;

namespace LOFAR
{

  Matrix<Complex> LofarWTerm::evaluate(const IPosition &shape,
                                       const DirectionCoordinate &coordinates,
                                       double w) const
  {
    if (w == 0) {
      return Matrix<Complex>(shape, 1.0);
    }

    Vector<double> resolution = coordinates.increment();
    double radius[2] = {0.5 * (shape[0]-1), 0.5 * (shape[1]-1)};
    double twoPiW = 2.0 * C::pi * w;

    Matrix<Complex> plane(shape, 0.0);
    for (int y = 0; y < shape[1]; ++y) {
      double m = resolution[1] * (y - radius[1]);
      double m2 = m * m;

      for (int x = 0; x < shape[0]; ++x) {
        double l = resolution[0] * (x - radius[0]);
        double lm2 = l * l + m2;

        if (lm2 < 1.0) {
          double phase = twoPiW * (sqrt(1.0 - lm2) - 1.0);
          plane(x, y) = Complex(cos(phase), sin(phase));
        }
      }
    }
    return plane;
  }

} // namespace LOFAR
