//#  FlagsMap.h: mmap operation for the data flag bits
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

#ifndef BBS3_FLAGSMAP_H
#define BBS3_FLAGSMAP_H

//# Includes
#include <BBS3/MMap.h>

namespace LOFAR
{

  // This class implements a mmap operation for the data flags which are bits.

  class FlagsMap
  {
  public:

    // Constructor
    FlagsMap (const string& fileName, MMap::protection prot);

    // Destructor
    ~FlagsMap();

    // Map the desired area
    void mapFile (int64 flagOffset, size_t nrFlags);

    // Unmap the last area
    void unmapFile();

    // Guarantee this memory range is resident in RAM (disable paging).
    // The memory range must be part of a mapped area.
    void lockMappedMemory();

    // Reenable paging 
    void unlockMappedMemory();

    // Get pointer to start of mapped region
    void* getStart();

    // Get starting bit of mapped flags.
    int getStartBit() const;

    // Get name of mapped file
    const string& getFileName();

    // Get the flag at the given position.
    bool FlagsMap::getFlag (uint pos);

    // Set the flag at the given position.
    void FlagsMap::setFlag (uint pos);

    // Clear the flag at the given position.
    void FlagsMap::clearFlag (uint pos);

  private:
    // Forbid copy constructor and assignment.
    // <group>
    FlagsMap (const FlagsMap&);
    FlagsMap& operator= (const FlagsMap&);
    // </group>

    MMap   itsMMap;
    uchar* itsFlags;             //# Mapped flags
    int    itsStartBit;          //# bitnr of the first flag
  };

  inline void* FlagsMap::getStart()
  {
    return itsFlags;
  }

  inline int FlagsMap::getStartBit() const
  {
    return itsStartBit;
  }

  inline bool FlagsMap::getFlag (uint pos)
  {
    uint realpos = pos + itsStartBit;
    uint index = realpos/8;
    return ((itsFlags[index] & (uchar(1) << (realpos%8)))  ==  0);
  }

} // namespace LOFAR

#endif
