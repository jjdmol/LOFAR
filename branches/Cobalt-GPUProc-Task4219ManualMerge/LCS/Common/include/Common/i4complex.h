//# i4complex.h: complex 2x4-bit integer type
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_COMMON_I4COMPLEX_H
#define LOFAR_COMMON_I4COMPLEX_H

#include <Common/lofar_iostream.h>


namespace LOFAR {
  namespace TYPES {
    class i4complex {
      public:
        i4complex() {}

	i4complex(double real, double imag) {
          int r = (int) rint(real);
          int i = (int) rint(imag);

          // clip to [-7..7] to center dynamic range around 0
          if (r <= -8) r = -7;
          if (i <= -8) i = -7;
          if (r >= 8)  r = 7;
          if (i >= 8)  i = 7;

	  value = (r & 0xF) | ((i & 0xF) << 4);
	}

	double real() const {
	  return ((signed char) (value << 4) >> 4); // extend sign
	}

	double imag() const {
	  return (value >> 4);
	}

  bool operator == (const i4complex &other) const {
    return value == other.value;
  }

  bool operator != (const i4complex &other) const {
    return value != other.value;
  }

	i4complex conj() const {
	  return i4complex(value ^ 0xF0);
	}

      private:
	i4complex(unsigned char value)
	: value(value) {}
	    
	// do not use bitfields for the real and imaginary parts, since the
	// allocation order is not portable between different compilers
	signed char value;
    };
  }

  inline TYPES::i4complex makei4complex(double real, double imag)
    { return TYPES::i4complex(real, imag); }

  inline double real(const TYPES::i4complex &v)
    { return v.real(); }

  inline double imag(const TYPES::i4complex &v)
    { return v.imag(); }

  inline TYPES::i4complex conj(const TYPES::i4complex &x)
    { return x.conj(); }

  inline ostream& operator<< (ostream& os, const TYPES::i4complex x)
    { os << '(' << real(x) << ',' << imag(x) << ')'; return os; }
}

#endif
