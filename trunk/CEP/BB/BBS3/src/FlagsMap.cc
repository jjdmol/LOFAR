//#  FlagsMap.cc: mmap operation for the data flag bits
//#
//#  Copyright (C) 2005
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

#include <lofar_config.h>
#include <BBS3/FlagsMap.h>

namespace LOFAR
{

  FlagsMap::FlagsMap (const string& fileName, MMap::protection prot)
    : itsMMap (fileName, prot),
      itsFlags(0)
  {}

  FlagsMap::~FlagsMap()
  {}

  void FlagsMap::mapFile (int64 flagOffset, size_t nrFlags)
  {
    // 8 flags per uchar are used, so get offset.
    int64 startOffset = flagOffset/8;
    size_t nrBytes = (nrFlags+7)/8;
    itsMMap.mapFile (startOffset, nrBytes);
    itsFlags = static_cast<uchar*>(itsMMap.getStart());
    itsStartBit = flagOffset%8;
  }

  void FlagsMap::unmapFile()
  {
    itsMMap.unmapFile();
  }

  void FlagsMap::lockMappedMemory()
  {
    itsMMap.lockMappedMemory();
  }

  void FlagsMap::unlockMappedMemory()
  {
    itsMMap.unlockMappedMemory();
  }

  void FlagsMap::setFlag (uint pos)
  {
    uint realpos = pos;
    uint index = realpos/8;
    itsFlags[index] |= (uchar(1) << (realpos%8));
  }

  void FlagsMap::clearFlag (uint pos)
  {
    uint realpos = pos;
    uint index = realpos/8;
    itsFlags[index] &= (~ (uchar(1) << (realpos%8)));
  }


} // namespace LOFAR
