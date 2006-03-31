//# lofar_complex.h:
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_COMMON_COMPLEX_H
#define LOFAR_COMMON_COMPLEX_H

// \file

//# ICC supports builtin complex for floating point types only.
//# GCC supports it for both floating point and integer types.
//# Both use __complex__ as the complex type specifier.
//# Other compilers might use _Complex, so define LOFAR_BUILTIN_COMPLEX
//# as such to use it.
//# Note that ComplexBuiltin.h uses __real__ and __imag__. Maybe they
//# also need different names for _Complex.
#if defined __INTEL_COMPILER || defined HAVE_BGL
# define LOFAR_BUILTIN_COMPLEXFP  __complex__
# include <Common/ComplexBuiltinFP.h>
# include <Common/ComplexStdInt.h>

#elif defined __GNUC__ && !defined __INSURE__
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
  using TYPES::i16complex;
  using TYPES::u16complex;
  using TYPES::fcomplex;
  using TYPES::dcomplex;

  i4complex makei4complex(i4complex &z) {
    return z;
  }

  i4complex makei4complex(i16complex &z) {
    return makei4complex(real(z), imag(z));
  }

  i4complex makei4complex(u16complex &z) {
    return makei4complex(real(z), imag(z));
  }

  i4complex makei4complex(fcomplex &z) {
    return makei4complex((int) real(z), (int) imag(z));
  }

  i4complex makei4complex(dcomplex &z) {
    return makei4complex((int) real(z), (int) imag(z));
  }

  i16complex makei16complex(i4complex &z) {
    return makei16complex(real(z), imag(z));
  }

  i16complex makei16complex(i16complex &z) {
    return z;
  }

  i16complex makei16complex(u16complex &z) {
    return makei16complex(real(z), imag(z));
  }

  i16complex makei16complex(fcomplex &z) {
    return makei16complex((int) real(z), (int) imag(z));
  }

  i16complex makei16complex(dcomplex &z) {
    return makei16complex((int) real(z), (int) imag(z));
  }

  u16complex makeu16complex(i4complex &z) {
    return makeu16complex(real(z), imag(z));
  }

  u16complex makeu16complex(i16complex &z) {
    return makeu16complex(real(z), imag(z));
  }

  u16complex makeu16complex(u16complex &z) {
    return z;
  }

  u16complex makeu16complex(fcomplex &z) {
    return makeu16complex((unsigned) real(z), (unsigned) imag(z));
  }

  u16complex makeu16complex(dcomplex &z) {
    return makeu16complex((unsigned) real(z), (unsigned) imag(z));
  }

  fcomplex makefcomplex(i4complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  fcomplex makefcomplex(i16complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  fcomplex makefcomplex(u16complex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  fcomplex makefcomplex(fcomplex &z) {
    return z;
  }

  fcomplex makefcomplex(dcomplex &z) {
    return makefcomplex((float) real(z), (float) imag(z));
  }

  dcomplex makedcomplex(i4complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  dcomplex makedcomplex(i16complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  dcomplex makedcomplex(u16complex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  dcomplex makedcomplex(fcomplex &z) {
    return makedcomplex((double) real(z), (double) imag(z));
  }

  dcomplex makedcomplex(dcomplex &z) {
    return z;
  }
}


#endif
