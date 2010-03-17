//# BlobOBufNull.cc: Output buffer for a blob using an onull
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

#include <Blob/BlobOBufNull.h>

namespace LOFAR {

BlobOBufNull::BlobOBufNull()
: itsSize (0),
  itsPos  (0)
{}

BlobOBufNull::~BlobOBufNull()
{}

uint64 BlobOBufNull::put (const void*, uint64 nbytes)
{
  itsPos += nbytes;
  if (itsPos > itsSize) {
    itsSize = itsPos;
  }
  return nbytes;
}

int64 BlobOBufNull::tellPos() const
{
  return itsPos;
}

int64 BlobOBufNull::setPos (int64 pos)
{
  itsPos = pos;
  if (itsPos > itsSize) {
    itsSize = itsPos;
  }
  return itsPos;
}

} // end namespace
