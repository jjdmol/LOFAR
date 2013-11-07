//# StringStream.cc: 
//#
//# Copyright (C) 2008
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
//# $Id: StringStream.cc 17976 2011-05-10 09:54:45Z mol $

#include <lofar_config.h>

#include <Stream/StringStream.h>
#include <Common/Thread/Cancellation.h>

#include <cstring>


namespace LOFAR {

StringStream::~StringStream()
{
  close();
}


size_t StringStream::tryRead(void *ptr, size_t size)
{
  Cancellation::point(); // keep behaviour consistent with real I/O streams

  if (!dataWritten.down(size))
    THROW(EndOfStreamException, "Stream has been closed");

  {
    ScopedLock sl(itsMutex);
    itsBuffer.read(static_cast<char*>(ptr), size);
  }

  return size;
}


size_t StringStream::tryWrite(const void *ptr, size_t size)
{
  Cancellation::point(); // keep behaviour consistent with real I/O streams

  {
    ScopedLock sl(itsMutex);
    itsBuffer.write(static_cast<const char*>(ptr), size);
  }

  dataWritten.up(size);

  return size;
}


void StringStream::close()
{
  dataWritten.noMore();
}

} // namespace LOFAR
