//# LofarTypes.h
//#
//# Copyright (C) 2002-2003
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

#ifndef LOFAR_COMMON_LOFARTYPES_H
#define LOFAR_COMMON_LOFARTYPES_H

// \file
//

#include <Common/LofarTypedefs.h>
#include <Common/lofar_complex.h>
//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Make sure we include <sys/types.h> or <qglobal.h> if availabe. These files
//# contain a number of typedefs for commonly used primitive data types. Some
//# of these will clash with our typedefs -- more specifically ushort, uint,
//# and ulong -- so we will use the ones in <sys/types.h> or <qglobal.h>
//# On some platforms sys/types.h does not include all definitions.
#ifdef HAVE_QT
# include <qglobal.h>    // contains typedef for uchar as well
#else
# include <sys/types.h>
#endif

namespace LOFAR
{
  //# Make the type names defined in LofarTypedefs.h available in the
  //# namespace LOFAR. 
  //# Note that QT defines uchar, uint, and ulong.
  //# Otherwise check if it's there.
#ifdef HAVE_QT
  using ::uchar;
  using ::uint;
  using ::ulong;

#else

  using TYPES::uchar;
# ifdef HAVE_USHORT
  using ::ushort;
# else
  using TYPES::ushort;
# endif  
# ifdef HAVE_UINT
  using ::uint;
# else
  using TYPES::uint;
# endif  
# ifdef HAVE_ULONG
  using ::ulong;
# else
  using TYPES::ulong;
# endif  

#endif

  using TYPES::longlong;
  using TYPES::ulonglong;
  using TYPES::ldouble;
 
  using TYPES::int8;
  using TYPES::uint8;
  using TYPES::int16;
  using TYPES::int32;
  using TYPES::int64;
  using TYPES::uint16;
  using TYPES::uint32;
  using TYPES::uint64;
}

#endif
