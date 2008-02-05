//# BlobIBufChar.cc: Input buffer for a blob using a plain pointer
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

#include <Blob/BlobIBufChar.h>
#include <Common/LofarLogger.h>
#include <string.h>

namespace LOFAR {

BlobIBufChar::BlobIBufChar (const void* buffer, uint size)
: itsBuffer ((const uchar*)buffer),
  itsSize   (size),
  itsPos    (0)
{}

BlobIBufChar::~BlobIBufChar()
{}

uint BlobIBufChar::get (void* buffer, uint nbytes)
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
  if (pos < itsSize) {
    itsPos = pos;
  } else {
    itsPos = itsSize;
  }
  return itsPos;
}

} // end namespace
