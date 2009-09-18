//# FileDescriptorBasedStream.cc: 
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
//# $Id$

#include <lofar_config.h>

#include <Stream/FileDescriptorBasedStream.h>
#include <Stream/SystemCallException.h>

#include <unistd.h>

#include <stdexcept>


namespace LOFAR {

FileDescriptorBasedStream::~FileDescriptorBasedStream()
{
  if (close(fd) < 0)
    throw SystemCallException("close", errno, THROW_ARGS);
}


void FileDescriptorBasedStream::read(void *ptr, size_t size)
{
  while (size > 0) {
    const ssize_t bytes = ::read(fd, ptr, size);
    
    if (bytes < 0)
      throw SystemCallException("read", errno, THROW_ARGS);

    if (bytes == 0) 
      throw EndOfStreamException("read", THROW_ARGS);

    size -= bytes;
    ptr   = static_cast<char *>(ptr) + bytes;
  }
}


void FileDescriptorBasedStream::write(const void *ptr, size_t size)
{
  while (size > 0) {
    const ssize_t bytes = ::write(fd, ptr, size);

    if (bytes < 0)
      throw SystemCallException("write", errno, THROW_ARGS);

    size -= bytes;
    ptr   = static_cast<const char *>(ptr) + bytes;
  }
}

} // namespace LOFAR
