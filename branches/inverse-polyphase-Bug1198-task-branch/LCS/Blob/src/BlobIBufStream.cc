//# BlobIBufStream.cc: Input buffer for a blob using an istream
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

#include <Blob/BlobIBufStream.h>
#include <iostream>

namespace LOFAR {

BlobIBufStream::BlobIBufStream (std::istream& is)
: itsStream (is.rdbuf())
{}

BlobIBufStream::~BlobIBufStream()
{}

uint64 BlobIBufStream::get (void* buffer, uint64 nbytes)
{
  return itsStream->sgetn ((char*)buffer, nbytes);
}

int64 BlobIBufStream::tellPos() const
{
  return itsStream->pubseekoff (0, std::ios::cur);
}

int64 BlobIBufStream::setPos (int64 pos)
{
  return itsStream->pubseekoff (pos, std::ios::beg);
}

} // end namespace
