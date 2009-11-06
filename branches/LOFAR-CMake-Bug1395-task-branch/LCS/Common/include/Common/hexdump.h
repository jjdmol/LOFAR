//# hexdump.h: create hexdump of given datablock
//#
//# Copyright (C) 2002-2004
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

#ifndef LOFAR_COMMON_HEXDUMP_H
#define LOFAR_COMMON_HEXDUMP_H

// \file
// Create hexdump of given datablock

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>


namespace LOFAR
{

  void hexdump (						 	 const void* buf, int32	nrBytes);
  void hexdump (FILE*				aFile, 	 const void* buf, int32	nrBytes);
  void hexdump (char*				aChrPtr, const void* buf, int32	nrBytes);
  void hexdump (string&			aString, const void* buf, int32	nrBytes);
  
} // namespace LOFAR

#endif
