//# BlobOBufNull.h: Output buffer for a blob using a plain pointer
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
      virtual uint put (const void* buffer, uint nbytes);
      
      // Get the position in the buffer.
      virtual int64 tellPos() const;
      
      // Set the position in the buffer.
      virtual int64 setPos (int64 pos);
      
      // Get the size of the data in the buffer.
      uint size() const;
      
    private:
      uint itsSize;
      uint itsPos;
    };

// @}

  inline uint BlobOBufNull::size() const
    {
      return itsSize;
    }

} // end namespace

#endif
