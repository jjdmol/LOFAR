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

#ifndef LOFAR_BBS3_MMAP_H
#define LOFAR_BBS3_MMAP_H

// \file MMap.h
// implements an mmap operation

//# Includes
#include <Common/lofar_string.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

// \ addtogroup BBS3
// @{
// This class implements a mmap operation.


class MMap
{
public:

  enum protection {Read, Write, ReWr};

  // Constructor.
  MMap (const string& fileName, protection prot);

  // Destructor.
  ~MMap();

  // Map the desired region.
  void mapFile (int64 offset, size_t size);

  // Flush the changed mapped data to the file.
  void flush();

  // Unmap the last region.
  void unmapFile();

  // Get offset of mapped region.
  int64 getOffset() const;

  // Get size of mapped region.
  size_t getSize() const;

  // Get pointer to start of mapped region.
  void* getStart();

  // Get name of mapped file.
  const string& getFileName() const;

  // Is the file writable?
  bool isWritable() const;
  

private:

  // Forbid copy constructor and assignment
  MMap (const MMap&);
  MMap& operator= (const MMap&);

  string itsFileName;       // File name
  int    itsFd;             // File descriptor
  size_t itsSize;           // Size of region to be mapped
  int64  itsOffset;         // Offset of region to be mapped
  size_t itsNrBytes;        // Actual number of mapped bytes 
  void*  itsPageStart;      // Pointer to the start of the mapped page(s)
  void*  itsPtr;            // Pointer to the start of requested map area
  protection itsProtection; // Read/write protection of mapped area

};

inline int64 MMap::getOffset() const
  { return itsOffset; }

inline size_t MMap::getSize() const
  { return itsSize; }

inline void* MMap::getStart()
  { return itsPtr; }

inline const string& MMap::getFileName() const
  { return itsFileName; }

inline bool MMap::isWritable() const
  { return itsProtection != MMap::Read; }

// @}

} // namespace LOFAR

#endif
