//# BlobIBufChar.cc: Input buffer for a blob using a plain pointer
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

#include <Blob/BlobIBufChar.h>
#include <Common/LofarLogger.h>
#include <string.h>

namespace LOFAR {

BlobIBufChar::BlobIBufChar (const void* buffer, uint64 size)
: itsBuffer ((const uchar*)buffer),
  itsSize   (size),
  itsPos    (0)
{}

BlobIBufChar::~BlobIBufChar()
{}

uint64 BlobIBufChar::get (void* buffer, uint64 nbytes)
{
  if (itsPos >= itsSize) {
    return 0;
  }
  if (itsPos+nbytes > itsSize) {
    nbytes = itsSize-itsPos;
  }
  memcpy (buffer, itsBuffer+itsPos, nbytes);
  itsPos += nbytes;
  return nbytes;
}

int64 BlobIBufChar::tellPos() const
{
  return itsPos;
}

int64 BlobIBufChar::setPos (int64 pos)
{
  ASSERT (pos >= 0);
  if (pos < (int32)itsSize) {
    itsPos = pos;
  } else {
    itsPos = itsSize;
  }
  return itsPos;
}

} // end namespace
