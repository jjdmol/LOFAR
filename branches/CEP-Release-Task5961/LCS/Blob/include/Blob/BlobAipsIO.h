//# BlobAipsIO.h: A Blob buffer for Aips++ ByteIO
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BLOB_BLOBAIPSIO_H
#define LOFAR_BLOB_BLOBAIPSIO_H

#include <casa/IO/ByteIO.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>

// \file
// A Blob buffer for Aips++ ByteIO

namespace LOFAR {

  // \ingroup %pkgname%
  // @{


  // This class makes it possible to use the AIPS++ AipsIO class on top of
  // a Blob buffer. In this way it is possible to write an AIPS++ LSQFit
  // object directly into a Blob buffer.
  // Note that it is even possible to mix AipsIO and Blob puts;
  // the same is true for gets.
  // Of course, the order of reading must be the same as the order of writing.

  class BlobAipsIO: public casa::ByteIO
  {
  public:
    // Construct from a Blob buffer.
    // <group>
    explicit BlobAipsIO (BlobOStream&);
    explicit BlobAipsIO (BlobIStream&);
    // </group>

    virtual ~BlobAipsIO();

    // Write the number of bytes.
    // When needed it expands the buffer.
    // An exception is thrown when the buffer is not writable or
    // when buffer expansion fails or is not possible.
    virtual void write (casa::uInt size, const void* buf);

    // Read \a size bytes from the memory buffer. Returns the number of
    // bytes actually read. Will throw an Exception (AipsError) if the
    // requested number of bytes could not be read unless throwException is set
    // to False. Will always throw an exception if the buffer is not readable
    // or the buffer pointer is at an invalid position.
    virtual casa::Int read (casa::uInt size, void* buf, bool throwException);

    // Get the length of the data in the buffer.
    virtual casa::Int64 length();

    // Is the byte stream readable?
    virtual bool isReadable() const;

    // Is the byte stream writable?
    virtual bool isWritable() const;

    // Is the byte stream seekable?
    virtual bool isSeekable() const;

private:
    // Make copy constructor and assignment private, so a user cannot
    // use them.
    // <group>
    BlobAipsIO (const BlobAipsIO&);
    BlobAipsIO& operator= (const BlobAipsIO&);
    // </group>

    // Reset the position pointer to the given value. It returns the
    // new position.
    // An exception is thrown when seeking before the start of the
    // buffer or when seeking past the end of a readonly buffer.
    // When seeking past the end of a writable buffer, the required
    // amount of bytes is added and initialized to zero.
    virtual casa::Int64 doSeek (casa::Int64 offset, casa::ByteIO::SeekOption);

    BlobOStream* itsOBuf;
    BlobIStream* itsIBuf;
  };

  // @}

}

#endif
