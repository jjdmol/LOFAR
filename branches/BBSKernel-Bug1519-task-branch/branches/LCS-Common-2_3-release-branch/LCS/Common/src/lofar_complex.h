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

// \file lofar_complex.h

//# ICC supports builtin complex for floating point types only.
//# GCC supports it for both floating point and integer types.
//# Both use __complex__ as the complex type specifier.
//# Other compilers might use _Complex, so define LOFAR_BUILTIN_COMPLEX
//# as such to use it.
//# Note that ComplexBuiltin.h uses __real__ and __imag__. Maybe they
//# also need different names for _Complex.
#if defined __INTEL_COMPILER
# define LOFAR_BUILTIN_COMPLEXFP  __complex__
# include <Common/ComplexBuiltinFP.h>
# include <Common/ComplexStdInt.h>

#elif defined __GNUC__ && !defined __INSURE__
# define LOFAR_BUILTIN_COMPLEXFP  __complex__
# define LOFAR_BUILTIN_COMPLEXINT __complex__
# include <Common/ComplexBuiltinFP.h>
# include <Common/ComplexBuiltinInt.h>
#elif defined HAVE_BGL
# define LOFAR_BUILTIN_COMPLEXFP __complex__
# include <Common/ComplexBuiltinFP.h>
# include <Common/ComplexStdInt.h>

#else
# include <Common/ComplexStdFP.h>
# include <Common/ComplexStdInt.h>

#endif


namespace LOFAR
{
  // Define complex types in LOFAR namespace.
  using TYPES::i16complex;
  using TYPES::u16complex;
  using TYPES::fcomplex;
  using TYPES::dcomplex;
}


#endif
