//# BlobStringType.h: Define type for a blob string
//#
//# Copyright (C) 2003
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

#ifndef LOFAR_BLOB_BLOBSTRINGTYPE_H
#define LOFAR_BLOB_BLOBSTRINGTYPE_H

// \file
// Define type for a blob string

#include <Common/Allocator.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // A BlobStringType object defines how a blob is represented in a BlobString.
  // See that class for more details.
  
  class BlobStringType
    {
    public:
      // Allocate as a string<uchar> or by using raw memory with a char*.
      // In case of char*, an allocator can be given (for heap or shared memory).
      // By default a heap allocator is used.
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
  
// @}

} // end namespace

#endif
