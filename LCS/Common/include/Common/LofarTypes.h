//# LofarTypes.h
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef COMMON_LOFARTYPES_H
#define COMMON_LOFARTYPES_H

#include <Common/LofarTypedefs.h>
#include <lofar_config.h>

// Make the type names defined in LofarTypedefs.h available in the
// global name space.
// Some types (e.g. uint) are sometimes defined in other header files.
// If so, they are used from there and are not put in the global namespace.

//# Define some unsigned only if not already done in /usr/include/sys/types.h
#if (HAVE_QT)
# include <qglobal.h>
#else
using LOFAR::TYPES::uchar;
# include <sys/types.h>
#endif
using LOFAR::TYPES::longlong;
using LOFAR::TYPES::ulonglong;
using LOFAR::TYPES::ldouble;
using LOFAR::TYPES::fcomplex;
using LOFAR::TYPES::dcomplex;

using LOFAR::TYPES::int8;
using LOFAR::TYPES::int16;
using LOFAR::TYPES::int32;
using LOFAR::TYPES::int64;
using LOFAR::TYPES::uint8;
using LOFAR::TYPES::uint16;
using LOFAR::TYPES::uint32;
using LOFAR::TYPES::uint64;

#endif
