//# BlobIBufChar.h: Input buffer for a blob using a plain pointer
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

#ifndef COMMON_BLOBIBUFCHAR_H
#define COMMON_BLOBIBUFCHAR_H

#include <Common/BlobIBuffer.h>

namespace LOFAR {

// This class is the BlobIBuffer that makes use of a character buffer,
// i.e. a buffer of signed or unsigned chars.

class BlobIBufChar : public BlobIBuffer
{
public:
  // Construct from a buffer with the given length.
  BlobIBufChar (const void* buffer, uint size);

  // Destructor.
  virtual ~BlobIBufChar();

  // Get the requested nr of bytes.
  virtual uint get (void* buffer, uint nbytes);

  // Get the position in the buffer.
  virtual int64 tellPos() const;

  // Set the position in the buffer and return the new position.
  virtual int64 setPos (int64 pos);

  // Get a typed pointer to an area in the buffer.
  // It is meant to be used in combination with BlobIStream::getSpace.
  template<typename U> const U* getPointer (uint position) const
    {
      DbgAssert(position < itsSize);
      return (const U*)(itsBuffer + position);
    }

private:
  const uchar* itsBuffer;
  uint         itsSize;
  uint         itsPos;
};

} // end namespace

#endif
