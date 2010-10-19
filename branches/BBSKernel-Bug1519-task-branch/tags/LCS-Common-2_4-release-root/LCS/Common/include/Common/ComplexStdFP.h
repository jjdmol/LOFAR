//# ComplexStdFP.h: Use std::complex for floating point types
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_COMMON_COMPLEXSTDFP_H
#define LOFAR_COMMON_COMPLEXSTDFP_H

// \file ComplexStdFP.h

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

  using std::real;
  using std::imag;
  using std::conj;
  using std::sin;
  using std::cos;
  using std::tan;
  using std::sinh;
  using std::cosh;
  using std::tanh;
  using std::exp;
  using std::sqrt;
  using std::pow;
  using std::log;
  using std::log;
  using std::abs;
  using std::arg;
}

#endif
