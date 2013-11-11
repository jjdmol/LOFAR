//# SharedMemoryStream.cc: 
//#
//# Copyright (C) 2012
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
//# $Id: SharedMemoryStream.cc 17976 2011-05-10 09:54:45Z mol $

#include <lofar_config.h>

#if defined USE_THREADS

#include <Stream/SharedMemoryStream.h>

#include <cstring>


namespace LOFAR {

SharedMemoryStream::~SharedMemoryStream()
{
}


size_t SharedMemoryStream::tryRead(void *ptr, size_t size)
{
  ScopedLock lock(readLock);

  writePosted.down();
  readSize = std::min(size, writeSize);
  memcpy(ptr, writePointer, readSize);

  readDone.up();
  return readSize;
}


size_t SharedMemoryStream::tryWrite(const void *ptr, size_t size)
{
  ScopedLock lock(writeLock);

  writePointer = ptr;
  writeSize    = size;
  writePosted.up();

  readDone.down();
  return readSize;
}

} // namespace LOFAR

#endif
