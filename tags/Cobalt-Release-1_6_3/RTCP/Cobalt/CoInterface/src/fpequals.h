//# fpequals.h: templated floating point comparison routines with epsilon
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef LOFAR_GPUPROC_FPEQUALS_H
#define LOFAR_GPUPROC_FPEQUALS_H

// @file
// Templated floating point comparison routines with epsilon

#include <cmath>
#include <complex>
#include <limits>

namespace LOFAR
{
  namespace Cobalt
  {
    // Type traits for Epsilon type to be used by fpEquals.
    // @{
    template<typename T>
    struct Epsilon
    {
      typedef T Type;
    };

    template<typename T>
    struct Epsilon< std::complex<T> >
    {
      typedef T Type;
    };
    // @}

    // Inexact floating point comparison routine. The 'tfpequals' test covers
    // these routines and has some more hints on reasonable epsilon values.
    // @c T can be float, double, long double, or complex of those.
    template <typename T>
    bool fpEquals(T x, T y, 
                  typename Epsilon<T>::Type eps = 
                  std::numeric_limits<typename Epsilon<T>::Type>::epsilon())
    {
      //# equality: shortcut, also needed to correctly handle inf args.
      if (x == y) return true;

      //# absolute
      typename Epsilon<T>::Type d_xy = std::abs(x - y);
      if (d_xy <= eps) return true;

      //# relative
      typename Epsilon<T>::Type d_max = std::max(std::abs(x), std::abs(y));
      return d_xy / d_max <= eps;
    }

  }
}

#endif

