//# BlobIBufStream.cc: Input buffer for a blob using an istream
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
