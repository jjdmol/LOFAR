//# BlobOBufStream.cc: Output buffer for a blob using an ostream
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

#include <Blob/BlobOBufStream.h>
#include <iostream>

namespace LOFAR {

BlobOBufStream::BlobOBufStream (std::ostream& os)
: itsStream (os.rdbuf())
{}

BlobOBufStream::~BlobOBufStream()
{}

uint64 BlobOBufStream::put (const void* buffer, uint64 nbytes)
{
  return itsStream->sputn ((const char*)buffer, nbytes);
}

int64 BlobOBufStream::tellPos() const
{
  return itsStream->pubseekoff (0, std::ios::cur);
}

int64 BlobOBufStream::setPos (int64 pos)
{
  return itsStream->pubseekoff (pos, std::ios::beg);
}

} // end namespace
