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

    // Write the number of bytes to the Blob stream.
    // An exception is thrown if the stream is not writable.
    //# The 2nd write and read function are defined for the read/write signatures
    //# in older casacore versions (pre v2.0).
    virtual void write (casa::Int64 size, const void* buf);
    virtual void write (casa::uInt size, const void* buf);

    // Read \a size bytes from the Blob stream. Returns the number of
    // bytes actually read. Will throw an Exception (AipsError) if the
    // requested number of bytes could not be read unless throwException is set
    // to False.
    virtual casa::Int64 read (casa::Int64 size, void* buf, bool throwException);
    virtual casa::Int read (casa::uInt size, void* buf, bool throwException);

    // Get the length of the Blob stream. Returns 0 if the stream is not seekable.
    virtual casa::Int64 length();

    // Is the Blob stream readable?
    virtual bool isReadable() const;

    // Is the Blob stream writable?
    virtual bool isWritable() const;

    // Is the Blob stream seekable?
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
    virtual casa::Int64 doSeek (casa::Int64 offset, casa::ByteIO::SeekOption);

    BlobOStream* itsOBuf;
    BlobIStream* itsIBuf;
  };

  // @}

}

#endif
