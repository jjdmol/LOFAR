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

// Define some unsigned only if not already done is /usr/include/sys/types.h
#if (HAVE_QT)
#include <qglobal.h>
#else
using LOFAR::uchar;
#if !defined(_SYS_TYPES_H) || !defined(__USE_MISC)
using LOFAR::ushort;
using LOFAR::uint;
using LOFAR::ulong;
#endif
#endif // QT
using LOFAR::longlong;
using LOFAR::ulonglong;
using LOFAR::ldouble;
using LOFAR::fcomplex;
using LOFAR::dcomplex;

using LOFAR::int16;
using LOFAR::int32;
using LOFAR::int64;
using LOFAR::uint16;
using LOFAR::uint32;
using LOFAR::uint64;

#endif
