//# BlobStringType.h: Define type for a blob string
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

#ifndef COMMON_BLOBSTRINGTYPE_H
#define COMMON_BLOBSTRINGTYPE_H

#include <Common/Allocator.h>


class BlobStringType
{
public:
  // Allocate the blob as a string<uchar> or using char*.
  // In case of char*, give an allocator (for normal or shared memory).
  explicit BlobStringType (bool useString,
			   const LOFAR::Allocator& = LOFAR::HeapAllocator());

  BlobStringType (const BlobStringType& that);

  ~BlobStringType();

  BlobStringType& operator= (const BlobStringType& that);

  // Use a string or not.
  bool useString() const
    { return itsUseString; }

  // Get the allocator
  LOFAR::Allocator& allocator()
    { return *itsAllocator; }

private:
  bool              itsUseString;
  LOFAR::Allocator* itsAllocator;
};


#endif
