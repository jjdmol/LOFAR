//# BlobStringType.cc: Define type for a blob string
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobStringType.h>

namespace LOFAR {

BlobStringType::BlobStringType (bool useString, const LOFAR::Allocator& alloc)
: itsUseString (useString),
  itsAllocator (alloc.clone())
{}

BlobStringType::~BlobStringType()
{
  delete itsAllocator;
}

BlobStringType::BlobStringType (const BlobStringType& that)
: itsUseString (that.itsUseString),
  itsAllocator (that.itsAllocator->clone())
{}

BlobStringType& BlobStringType::operator= (const BlobStringType& that)
{
  if (this != &that) {
    delete itsAllocator;
    itsUseString = that.itsUseString;
    itsAllocator = that.itsAllocator->clone();
  }
  return *this;
}

} // end namespace
