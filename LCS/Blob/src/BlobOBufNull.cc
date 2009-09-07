//# BlobOBufNull.cc: Output buffer for a blob using an onull
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
