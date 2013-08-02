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
#include <casa/BasicSL/Constants.h>
#include <algorithm>

using namespace casa;

namespace LOFAR
{

  Matrix<Complex> LofarWTerm::evaluate(const IPosition &shape,
                                       const Vector<Double>& resolution,
                                       double w) const
  {
    
    Matrix<Complex> plane(shape);
    evaluate (plane.data(), shape[0], shape[1], resolution, w);
    return plane;
  }


  void LofarWTerm::evaluate(Complex* buffer,
                            int nx, int ny,
                            const Vector<Double>& resolution,
                            double w) const
  {
    if (w == 0) {
      std::fill (buffer, buffer+nx*ny, Complex(1.0));
    } else {

      double radius[2] = {0.5 * (nx-1), 0.5 * (ny-1)};
      double twoPiW = 2.0 * C::pi * w;

      for (int y = 0; y < ny; ++y) {
        double m = resolution[1] * (y - radius[1]);
        double m2 = m * m;

        for (int x = 0; x < nx; ++x) {
          double l = resolution[0] * (x - radius[0]);
          double lm2 = l * l + m2;

          if (lm2 < 1.0) {
            double phase = twoPiW * (sqrt(1.0 - lm2) - 1.0);
            *buffer = Complex(cos(phase), sin(phase));
          }
          buffer++;
        }
      }
    }
  }

  Complex LofarWTerm::evaluate_pixel(int x, int y, int nx, int ny,
				     const Vector<Double>& resolution,
				     double w) const
  {
    // if (w == 0) {
    //   return Complex(1.0);
    // } else {

      double radius[2] = {0.5 * (nx), 0.5 * (ny)};
      double twoPiW = 2.0 * C::pi * w;

      double m = resolution[1] * (y - radius[1]);
      double m2 = m * m;
      
      double l = resolution[0] * (x - radius[0]);
      double lm2 = l * l + m2;
      
      double phase = twoPiW * (sqrt(1.0 - lm2) - 1.0);
	  
      return Complex(cos(phase), sin(phase));
      //}
  }

} // namespace LOFAR
