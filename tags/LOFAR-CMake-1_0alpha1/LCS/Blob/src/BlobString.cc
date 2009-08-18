//# BlobString.h: Blob buffer that can be a string<uchar> or char
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobString.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

BlobString::BlobString (bool useString, size_t capacity,
			bool canIncreaseCapacity, uint alignment)
: itsAllocator (BlobStringType(useString, LOFAR::HeapAllocator())),
  itsCapacity  (0),
  itsSize      (0),
  itsCanIncr   (true),
  itsAlignment (alignment),
  itsBuffer    (0)
{
  // Make sure alignment is power of 2.
  if (alignment > 1) {
    ASSERTSTR ((alignment & (alignment-1)) == 0,
	       "Blob alignment " << alignment << " must be a power of 2");
  }
  reserve (capacity);
  itsCanIncr = canIncreaseCapacity;
}

BlobString::BlobString (const BlobStringType& allocator, size_t capacity,
			bool canIncreaseCapacity, uint alignment)
: itsAllocator (allocator),
  itsCapacity  (0),
  itsSize      (0),
  itsCanIncr   (true),
  itsAlignment (alignment),
  itsBuffer    (0)
{
  // Make sure alignment is power of 2.
  if (alignment > 1) {
    ASSERT ((alignment & (alignment-1)) == 0);
  }
  reserve (capacity);
  itsCanIncr = canIncreaseCapacity;
}

BlobString::~BlobString()
{
  if (! itsAllocator.useString()) {
    itsAllocator.allocator().deallocate (itsBuffer);
  }
}

void BlobString::reserve (size_t newSize)
{
  if (newSize > itsCapacity) {
    ASSERTSTR (itsCanIncr, "This BlobString cannot increase its capacity");
    if (itsAllocator.useString()) {
      itsString.reserve (newSize);
      itsBuffer = const_cast<uchar*>(itsString.data());
      itsCapacity = itsString.capacity();
    } else {
      void* newbuf = itsAllocator.allocator().allocate (newSize, itsAlignment);
      ASSERTSTR (newbuf, "BlobString could not allocate " << newSize
		 << " bytes");
      memcpy (newbuf, itsBuffer, itsSize);
      itsAllocator.allocator().deallocate (itsBuffer);
      itsBuffer = newbuf;
      itsCapacity = newSize;
    }
  }
}

void BlobString::resize (size_t newSize)
{
  if (newSize != itsSize) {
    if (newSize > itsCapacity) {
      reserve (newSize);
    }
    if (itsAllocator.useString()) {
      itsString.resize (newSize);
      itsBuffer = const_cast<uchar*>(itsString.data());
    }
    itsSize = newSize;
  }
}

std::basic_string<uchar>& BlobString::getString()
{
  ASSERTSTR (itsAllocator.useString(), "BlobString has no string");
  return itsString;
}

} // end namespace
