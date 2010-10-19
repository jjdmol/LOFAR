//# ComplexStdInt.h: Use std::complex for integer types
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

#ifndef LOFAR_COMMON_COMPLEXSTDINT_H
#define LOFAR_COMMON_COMPLEXSTDINT_H

// \file ComplexStdInt.h

#include <complex>
#include <Common/LofarTypedefs.h>

namespace LOFAR {

  namespace TYPES {
    typedef std::complex<int16>  i16complex;
    typedef std::complex<uint16> u16complex;
  }

  inline TYPES::i16complex makei16complex (TYPES::uint16 re, TYPES::uint16 im)
    { return TYPES::i16complex(re,im); }
  inline TYPES::u16complex makeu16complex (TYPES::uint16 re, TYPES::uint16 im)
    { return TYPES::u16complex(re,im); }

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
