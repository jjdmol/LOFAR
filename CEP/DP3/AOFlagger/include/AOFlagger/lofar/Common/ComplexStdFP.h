//# ComplexStdFP.h: Use std::complex for floating point types
//#
//# Copyright (C) 2005
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
//# $Id: ComplexStdFP.h 14057 2009-09-18 12:26:29Z diepen $

#ifndef LOFAR_COMMON_COMPLEXSTDFP_H
#define LOFAR_COMMON_COMPLEXSTDFP_H

// \file

#include <complex>

namespace LOFAR {

  namespace TYPES {
    typedef std::complex<float> fcomplex;
    typedef std::complex<double> dcomplex;
  }

  inline TYPES::fcomplex makefcomplex (float re, float im)
    { return TYPES::fcomplex(re,im); }
  inline TYPES::dcomplex makedcomplex (double re, double im)
    { return TYPES::dcomplex(re,im); }

  using std::abs;
  using std::arg;
  using std::conj;
  using std::cos;
  using std::cosh;
  using std::exp;
  using std::imag;
  using std::log;
  using std::log10;
  using std::norm;
  using std::polar;
  using std::pow;
  using std::real;
  using std::sin;
  using std::sinh;
  using std::sqrt;
  using std::tan;
  using std::tanh;
}

#endif
