//# DataFormat.h: Get the data format (endian type)
//#
//#  Copyright (C) 2003
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

#ifndef COMMON_DATAFORMAT_H
#define COMMON_DATAFORMAT_H


#include <config.h>

// This file defines the data format on a machine.
// Currently only little and big endian is possible with floating point
// numbers as IEEE and characters in the ASCII representation.
// It is used in the Blob classes and the DataConvert functions.

namespace LOFAR
{
  enum DataFormat {LittleEndian=0, BigEndian=1};

  // Get the endian type on this machine.
  inline DataFormat dataFormat()
#if defined(WORDS_BIGENDIAN)
   {return BigEndian; }
#else
   {return LittleEndian; }
#endif
}


#endif
