//# MSWriterNull: a null MSWriter
//#
//#  Copyright (C) 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: $

#include <lofar_config.h>

#include <Storage/MSWriterFile.h>
#include <Interface/SmartPtr.h>

#include <sys/types.h>
#include <fcntl.h>


namespace LOFAR {
namespace RTCP {


FastFileStream::FastFileStream(const string &name, int flags, int mode)
:
  FileStream(name.c_str(), flags | O_DIRECT | O_SYNC, mode),
  bufsize(0),
  buffer(0),
  remainder(0)
{
  // alignment must be a power of two for easy calculations
  ASSERT( (alignment & (alignment-1)) == 0 );

  // alignment must be a multiple of sizeof(void*) for posix_memalign to work
  ASSERT( alignment % sizeof(void*) == 0 );
}

FastFileStream::~FastFileStream()
{
  if (remainder) {
    // pad with zeroes
    ensureBuffer(alignment);
    memset(buffer + remainder, 0, alignment - remainder);
    forceWrite(buffer, alignment);
  }
} 

void FastFileStream::ensureBuffer(size_t newsize)
{
  if (newsize <= bufsize)
    return;

  void *buf;

  if (posix_memalign(&buf, alignment, newsize) != 0)
    THROW( StorageException, "Not enough memory to allocate " << newsize << " bytes for fast writing");

  if (remainder) {
    ASSERT( buffer.get() );
    ASSERT( newsize >= remainder );

    memcpy(buf, buffer.get(), remainder);
  }

  buffer = static_cast<char*>(buf); // SmartPtr will take care of deleting the old buffer
  bufsize = newsize;
}     

void FastFileStream::forceWrite(const void *ptr, size_t size)
{
  // emulate Stream::write using FileStream::write to make sure all bytes are written
  while (size > 0) {
    ASSERT( (size & (alignment-1)) == 0 );
    ASSERT( (reinterpret_cast<size_t>(ptr) & (alignment-1)) == 0 );

    size_t bytes = FileStream::tryWrite(ptr, size);

    size -= bytes;
    ptr   = static_cast<const char *>(ptr) + bytes;
  }
}

size_t FastFileStream::tryWrite(const void *ptr, size_t size)
{
  const size_t orig_size = size;

  if (!remainder && (reinterpret_cast<size_t>(ptr) & (alignment-1)) == 0) {
    // pointer is aligned and we can write from it immediately

    ensureBuffer(alignment); // although remainder is enough, we want to avoid reallocating every time remainder grows slightly

    // save the remainder
    remainder = size & (alignment-1);
    memcpy(buffer.get(), static_cast<const char*>(ptr) + size - remainder, remainder);

    // write bulk
    forceWrite(ptr, size - remainder);
  } else {
    // not everything is aligned or there is a remainder -- use the buffer

    // move data to our buffer, and recompute new sizes
    ensureBuffer(alignment + size); // although remainder + size is enough, we want to avoid reallocating every time remainder grows slightly
    memcpy(buffer.get() + remainder, ptr, size);

    size += remainder;
    remainder = size & (alignment-1);

    // write bulk
    forceWrite(buffer.get(), size - remainder);

    // move remainder to the front
    memmove(buffer.get(), buffer.get() + size - remainder, remainder);
  }

  // lie about how many bytes we've written, since we might be caching
  // a remainder which we can't write to disk.
  return orig_size;
}


MSWriterFile::MSWriterFile (const string &msName, bool oldFileFormat)
:
 itsFile(msName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH),
 itsOldFileFormat(oldFileFormat) // true if the header is just the sequence number padded to 512 bytes
{
}


MSWriterFile::~MSWriterFile()
{
}


void MSWriterFile::write(StreamableData *data)
{
  uint32_t magicValue = 0; // initialise to satisfy compiler

  if (itsOldFileFormat) {
    ASSERT( FastFileStream::alignment == 512 );

    // a hack to get the sequence number as the first 4 bytes, replacing the magic value.
    magicValue = data->peerMagicNumber;
    data->peerMagicNumber = data->sequenceNumber(true);
  }

  data->write(&itsFile, true, FastFileStream::alignment);

  if (itsOldFileFormat)
    data->peerMagicNumber = magicValue;
}


} // namespace RTCP
} // namespace LOFAR

