//# BlobOBuffer.h: Abstract base class for output buffer for a blob
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

#ifndef LOFAR_BLOB_BLOBOBUFFER_H
#define LOFAR_BLOB_BLOBOBUFFER_H

// \file
// Abstract base class for output buffer for a blob

#include <Common/LofarTypes.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{
  
  // BlobOBuffer is the abstract base class for the sink of a
  // BlobOStream object. In this way the destination of a BlobOStream
  // can be chosen as needed. Currently three sink types are possible:
  // <ul>
  // <li> BlobOBufStream uses an ostream as the sink. It is mainly
  //      meant to write blobs into a file, socket, etc.
  // <li> BlobOBufVector<T> uses a vector<T> (where T is char or uchar)
  //      as the sink. It is mainly meant for creating blobs to be
  //      stored using the classes in the LCS Persistent Object Layer.
  // <li> BlobOBufChar uses a plain C-array as the sink. It serves as
  //      a helper class for BlobOBufVector, but can also be used in itself.
  // </ul>
  
  class BlobOBuffer
    {
    public:
      BlobOBuffer()
	{};
      
      virtual ~BlobOBuffer()
	{};
      
      // Put the requested nr of bytes.
      virtual uint put (const void* buffer, uint nbytes) = 0;
      
      // Get the position in the stream.
      // -1 is returned if the stream is not seekable.
      virtual int64 tellPos() const = 0;
      
      // Set the position in the stream.
      // It returns the new position which is -1 if the stream is not seekable.
      virtual int64 setPos (int64 pos) = 0;
    };

  // @}

} // end namespace

#endif
