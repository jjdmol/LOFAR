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

#include <Common/BlobString.h>
#include <Common/Debug.h>


BlobString::BlobString (const BlobStringType& allocator, size_t capacity)
: itsAllocator (allocator),
  itsCapacity  (0),
  itsSize      (0),
  itsChars     (0)
{
  reserve (capacity);
}

BlobString::~BlobString()
{
  if (! itsAllocator.useString()) {
    itsAllocator.allocator().deallocate (itsChars);
  }
}

void BlobString::reserve (size_t newSize)
{
  if (newSize > itsCapacity) {
    if (itsAllocator.useString()) {
      itsString.reserve (newSize);
      itsChars = const_cast<uchar*>(itsString.data());
      itsCapacity = itsString.capacity();;
    } else {
      void* data = itsAllocator.allocator().allocate (newSize);
      AssertStr (data, "BlobString could not allocate " << newSize
		       << " bytes");
      memcpy (data, itsChars, itsSize);
      itsAllocator.allocator().deallocate (itsChars);
      itsChars = data;
      itsCapacity = newSize;
    }
  }
}

void BlobString::resize (size_t newSize)
{
  DbgAssert (newSize <= itsCapacity);
  if (newSize > itsSize) {
    if (itsAllocator.useString()) {
      itsString.resize (newSize);
    }
    itsSize = newSize;
  }
}

const std::basic_string<uchar>& BlobString::getString() const
{
  AssertStr (itsAllocator.useString(), "BlobString has no string");
  return itsString;
}
