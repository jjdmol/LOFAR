//# lofar_complex.h:
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_COMMON_COMPLEX_H
#define LOFAR_COMMON_COMPLEX_H

// \file

//# Put sin, etc. in LOFAR namespace.
#include <Common/lofar_math.h>

//# ICC supports builtin complex for floating point types only.
//# GCC supports it for both floating point and integer types.
//# Both use __complex__ as the complex type specifier.
//# Other compilers might use _Complex, so define LOFAR_BUILTIN_COMPLEX
//# as such to use it.
//# Note that ComplexBuiltin.h uses __real__ and __imag__. Maybe they
//# also need different names for _Complex.
//# Note: include StdInt and BuiltinFP in this order, otherwise the gcc
//# compiler complains that conj already exists in LOFAR namespace.
#if defined __INTEL_COMPILER || defined HAVE_BGL
# define LOFAR_BUILTIN_COMPLEXFP  __complex__
# include <Common/ComplexStdInt.h>
# include <Common/ComplexBuiltinFP.h>

#elif defined __GNUC__ && __GNUC__ < 4 && !defined __INSURE__
# define LOFAR_BUILTIN_COMPLEXFP  __complex__
# define LOFAR_BUILTIN_COMPLEXINT __complex__
# include <Common/ComplexBuiltinFP.h>
# include <Common/ComplexBuiltinInt.h>

#else
# include <Common/ComplexStdFP.h>
# include <Common/ComplexStdInt.h>

#endif

#include <Common/i4complex.h>

namespace LOFAR
{
  // Define complex types in LOFAR namespace.
  using TYPES::i4complex;
  using TYPES::i8complex;
  using TYPES::i16complex;
  using TYPES::u16complex;
  using TYPES::fcomplex;
  using TYPES::dcomplex;

  inline static i4complex makei4complex(const i4complex &z) {
    return z;
  }

  inline static i4complex makei4complex(const i8complex &z) {
    return makei4complex(real(z) - .5, imag(z) - .5);
  }

  inline static i4complex makei4complex(const i16complex &z) {
    return makei4complex(real(z) - .5, imag(z) - .5);
  }

  inline static i4complex makei4complex(const u16complex &z) {
    return makei4complex(real(z) - .5, imag(z) - .5);
  }

  inline static i4complex makei4complex(const fcomplex &z) {
    return makei4complex(real(z), imag(z));
  }

  inline static i4complex makei4complex(const dcomplex &z) {
    return makei4complex(real(z), imag(z));
  }

  inline static i8complex makei8complex(const i4complex &z) {
    return makei8complex((int) real(z), (int) imag(z));
  }

  inline static i8complex makei8complex(const i8complex &z) {
    return z;
  }

  inline static i8complex makei8complex(const i16complex &z) {
    return makei8complex(real(z), imag(z));
  }

  inline static i8complex makei8complex(const u16complex &z) {
    return makei8complex(real(z), imag(z));
  }

  inline static i8complex makei8complex(const fcomplex &z) {
    return makei8complex((int) real(z), (int) imag(z));
  }

  inline static i8complex makei8complex(const dcomplex &z) {
    return makei8complex((int) real(z), (int) imag(z));
  }

  inline static i16complex makei16complex(const i4complex &z) {
    return makei16complex((int) real(z), (int) imag(z));
  }

  inline static i16complex makei16complex(const i8complex &z) {
    return makei16complex(real(z), imag(z));
  }

  inline static i16complex makei16complex(const i16complex &z) {
    return z;
  }

  inline static i16complex makei16complex(const u16complex &z) {
    return makei16complex(real(z), imag(z));
  }

  inline static i16complex makei16complex(const fcomplex &z) {
    return makei16complex((int) real(z), (int) imag(z));
  }

  inline static i16complex makei16complex(const dcomplex &z) {
    return makei16complex((int) real(z), (int) imag(z));
  }

  inline static u16complex makeu16complex(const i4complex &z) {
    return makeu16complex((int) real(z), (int) imag(z));
  }

  inline static u16complex makeu16complex(const i8complex &z) {
    return makeu16complex(real(z), imag(z));
  }

  inline static u16complex makeu16complex(const i16complex &z) {
    return makeu16complex(real(z), imag(z));
  }

  inline static u16complex makeu16complex(const u16complex &z) {
    return z;
  }

  inline static u16complex makeu16complex(const fcomplex &z) {
    return makeu16complex((unsigned) real(z), (unsigned) imag(z));
  }

  inline static u16complex makeu16complex(const dcomplex &z) {
    return makeu16complex((unsigned) real(z), (unsigned) imag(z));
  }

  inline static fcomplex makefcomplex(const i4complex &z) {
    return makefcomplex(real(z), imag(z));
  }

  inline static fcomplex makefcomplex(const i8complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  inline static fcomplex makefcomplex(const i16complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  inline static fcomplex makefcomplex(const u16complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  inline static fcomplex makefcomplex(const fcomplex &z) {
    return z;
  }

  inline static fcomplex makefcomplex(const dcomplex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  inline static dcomplex makedcomplex(const i4complex &z) {
    return makedcomplex(real(z), imag(z));
  }

  inline static dcomplex makedcomplex(const i8complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  inline static dcomplex makedcomplex(const i16complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  inline static dcomplex makedcomplex(const u16complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  inline static dcomplex makedcomplex(const fcomplex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  inline static dcomplex makedcomplex(const dcomplex &z) {
    return z;
  }
}


#endif
