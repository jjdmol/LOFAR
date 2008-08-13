//#  deprecated_warning.h: Issue a "deprecated" warning.
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_MATH_DEPRECATED_WARNING_H
#define LOFAR_MATH_DEPRECATED_WARNING_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Issue a "deprecated" warning.

#ifdef __DEPRECATED
#warning LCSMath is deprecated. It will be phased out in the near future. \
         To disable this warning use -Wno-deprecated.
#endif

#endif
