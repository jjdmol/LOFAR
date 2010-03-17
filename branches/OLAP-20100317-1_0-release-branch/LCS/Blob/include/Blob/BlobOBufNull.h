//# BlobOBufNull.h: Output buffer for a blob using a plain pointer
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

#ifndef LOFAR_BLOB_BLOBOBUFNULL_H
#define LOFAR_BLOB_BLOBOBUFNULL_H

// \file
// Output buffer for a blob using a plain pointer

#include <Blob/BlobOBuffer.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{
  
  // This class is the BlobOBuffer that makes use of a null buffer.
  // It can be used to determine the length of a blob.
  
  class BlobOBufNull : public BlobOBuffer
    {
    public:
      // Construct a dynamic buffer with the given initial length.
      BlobOBufNull();
      
      // Destructor.
      virtual ~BlobOBufNull();
      
      // Put the requested nr of bytes.
      virtual uint64 put (const void* buffer, uint64 nbytes);
      
      // Get the position in the buffer.
      virtual int64 tellPos() const;
      
      // Set the position in the buffer.
      virtual int64 setPos (int64 pos);
      
      // Get the size of the data in the buffer.
      uint64 size() const;
      
    private:
      uint64 itsSize;
      uint64 itsPos;
    };

// @}

  inline uint64 BlobOBufNull::size() const
    {
      return itsSize;
    }

} // end namespace

#endif
