//#  MMap.h: implements a mmap operation
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
//#  $ $

#ifndef PSS3_MMAP_H
#define PSS3_MMAP_H

#include <Common/lofar_string.h>

//# Includes

#include <Common/LofarTypes.h>

namespace LOFAR
{

/** 
    This class implements a mmap operation
*/

class MMap
{
 public:

  enum protection{Read, Write, ReWr};

  // Constructor
  MMap (const string& fileName, protection prot);

  // Destructor
  virtual ~MMap();

  // Map the desired area
  void mapFile(long long startOffset, size_t nrBytes);

  // Unmap the last area
  void unmapFile();

  // Guarantee this memory range is resident in RAM (disable paging). The memory range
  // must be part of a mapped area.
  void lockMappedMemory(void* addr, size_t nrBytes);

  // Reenable paging 
  void unlockMappedMemory();

  // Get pointer to start of mapped region
  void* getStart();

  // Get name of mapped file
  const string& getFileName();

 private:

  // Forbid copy constructor and assignment
  MMap (const MMap&);
  MMap& operator= (const MMap&);

  string itsFileName;       // File name
  int    itsFd;             // File descriptor
  size_t itsNrBytes;        // Number of mapped bytes 
  void*  itsPageStart;      // Pointer to the start of the mapped page(s)
  void*  itsPtr;            // Pointer to the start of requested map area
  protection itsProtection; // Read/write protection of mapped area

  void*  itsLockAddr;       // Start address for memory lock
  size_t itsLockBytes;      // Number of memory locked bytes
};

inline void* MMap::getStart()
  { return itsPtr; }

inline const string& MMap::getFileName()
   { return itsFileName; }
} // namespace LOFAR

#endif
