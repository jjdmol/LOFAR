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

#ifndef COMMON_BLOBSTRING_H
#define COMMON_BLOBSTRING_H

#include <Common/LofarTypes.h>
#include <Common/BlobStringType.h>
#include <Common/BlobStringTraits.h>
#include <string>


class BlobString
{
public:
  // Create a data buffer with the given size.
  // The buffer is a char* or a string<uchar> depending on the allocator
  // object.
  explicit BlobString (const BlobStringType&, size_t size=0);

  ~BlobString();

  // Get the size.
  size_t size() const
    { return itsSize; }

  // Resize the data buffer.
  void resize (size_t newSize);

  // Get the capacity.
  size_t capacity() const
    { return itsCapacity; }

  // Set the capacity.
  void reserve (size_t newSize);

  // Get a pointer to the data.
  // <group>
  const char* data() const
    { return (const char*)itsChars; }
  char* data()
    { return (char*)itsChars; }
  // </group>

  // Get the data as a string of uchar.
  // It throws an exception if not allocated that way.
  const std::basic_string<uchar>& getString() const;

private:
  // Forbid copy constructor and assignment.
  // <group>
  BlobString (const BlobString&);
  BlobString& operator= (const BlobString&);
  // </group>


  BlobStringType           itsAllocator;
  size_t                   itsCapacity;
  size_t                   itsSize;
  void*                    itsChars;
  std::basic_string<uchar> itsString;
};


#endif
