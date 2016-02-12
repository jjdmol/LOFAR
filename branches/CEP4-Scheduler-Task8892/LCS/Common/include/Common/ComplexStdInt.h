//# ComplexStdInt.h: Use std::complex for integer types
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
//# $Id$

#ifndef LOFAR_COMMON_COMPLEXSTDINT_H
#define LOFAR_COMMON_COMPLEXSTDINT_H

// \file

#include <complex>
#include <Common/LofarTypedefs.h>

namespace LOFAR {

  namespace TYPES {
    typedef std::complex<int8>   i8complex;
    typedef std::complex<int16>  i16complex;
    typedef std::complex<uint16> u16complex;
  }

  inline TYPES::i8complex makei8complex (TYPES::int8 re, TYPES::int8 im)
    { return TYPES::i8complex(re,im); }
  inline TYPES::i16complex makei16complex (TYPES::uint16 re, TYPES::uint16 im)
    { return TYPES::i16complex(re,im); }
  inline TYPES::u16complex makeu16complex (TYPES::uint16 re, TYPES::uint16 im)
    { return TYPES::u16complex(re,im); }

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
