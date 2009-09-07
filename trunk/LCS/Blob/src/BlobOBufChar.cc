//# BlobOBufChar.cc: Input buffer for a blob using a plain pointer
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

#include <Blob/BlobOBufChar.h>
#include <Blob/BlobException.h>
#include <Common/LofarLogger.h>
#include <string.h>

namespace LOFAR {

BlobOBufChar::BlobOBufChar (uint64 initialSize, uint64 expandSize)
: itsBuffer       (0),
  itsSize         (0),
  itsPos          (0),
  itsReservedSize (initialSize),
  itsExpandSize   (expandSize),
  itsIsOwner      (true)
{
  if (initialSize > 0) {
    itsBuffer = new uchar[initialSize];
  }
}

BlobOBufChar::BlobOBufChar (void* buffer, uint64 size, uint64 expandSize,
			    uint64 start, bool takeOver)
: itsBuffer       ((uchar*)buffer),
  itsSize         (start),
  itsPos          (start),
  itsReservedSize (size),
  itsExpandSize   (expandSize),
  itsIsOwner      (takeOver)
{
  ASSERT (start <= size);
  ASSERT (size == 0  ||  (size > 0  &&  buffer != 0));
}

BlobOBufChar::~BlobOBufChar()
{
  if (itsIsOwner) {
    delete [] itsBuffer;
  }
}

void BlobOBufChar::clear()
{
  // Let derived class set its size to 0.
  doExpand (0, 0);
  itsSize = 0;
  itsPos  = 0;
}

uint64 BlobOBufChar::put (const void* buffer, uint64 nbytes)
{
  // Expand the buffer if needed (and possible).
  if (! resizeIfNeeded (itsPos+nbytes)) {
    return 0;
  }
  // Copy the data and set new position and size.
  memcpy (itsBuffer + itsPos, buffer, nbytes);
  itsPos += nbytes;
  if (itsPos > itsSize) {
    itsSize = itsPos;
  }
  return nbytes;
}

int64 BlobOBufChar::tellPos() const
{
  return itsPos;
}

int64 BlobOBufChar::setPos (int64 pos)
{
  // It is possible to seek past the end of the buffer.
  // This means that the buffer size increases.
  // Expand the buffer if needed.
  // Initialize the new buffer positions with zeroes.
  if (pos > itsSize) {
    if (! resizeIfNeeded (pos)) {
      THROW(BlobException,
	    "BlobOBufChar::setPos - buffer cannot be expanded");
    }
    memset(itsBuffer + itsSize, 0, pos - itsSize);
    itsSize = pos;
  }
  itsPos = pos;
  return pos;
}

bool BlobOBufChar::expand (uint64 newSize)
{
  uint64 minsz = itsReservedSize;
  if (newSize > minsz) {
    if (itsExpandSize == 0) {
      return false;                  // cannot expand
    }
    minsz = itsReservedSize + itsExpandSize;
    if (newSize > minsz) {
      minsz = newSize;
    }
  }
  doExpand (minsz, newSize);
  itsReservedSize = minsz;
  return true;
}

void BlobOBufChar::doExpand (uint64 newReservedSize, uint64)
{
  if (newReservedSize > itsReservedSize) {
    // Allocate new buffer, copy contents and delete old buffer (if possible).
    uchar* newBuffer = new uchar[newReservedSize];
    // Copy the old contents (if any).
    if (itsBuffer != 0) {
      memcpy (newBuffer, itsBuffer, itsSize);
      if (itsIsOwner) {
	delete [] itsBuffer;
      }
    }
    itsBuffer  = newBuffer;
    itsIsOwner = true;
  }
}

void BlobOBufChar::reserve (uint64 newReservedSize)
{
  if (newReservedSize > itsReservedSize) {
    doExpand (newReservedSize, itsSize);
    itsReservedSize = newReservedSize;
  }
}

void BlobOBufChar::resize (uint64 newSize)
{
  doExpand (newSize, newSize);
  itsSize = newSize;
}

} // end namespace
