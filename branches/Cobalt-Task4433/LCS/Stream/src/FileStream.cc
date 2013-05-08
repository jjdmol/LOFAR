//# FileStream.cc: 
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

#include <Common/SystemCallException.h>
#include <Stream/FileStream.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {

FileStream::FileStream(const std::string &name)
{
  if ((fd = open(name.c_str(), O_RDONLY)) < 0)
    throw SystemCallException(std::string("open ") + name, errno, THROW_ARGS);
}


FileStream::FileStream(const std::string &name, int mode)
{
  if ((fd = open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode)) < 0)
    throw SystemCallException(std::string("open ") + name, errno, THROW_ARGS);
}


FileStream::FileStream(const std::string &name, int flags, int mode)
{
  if ((fd = open(name.c_str(), flags, mode)) < 0) 
    throw SystemCallException(std::string("open ") + name, errno, THROW_ARGS);
}

FileStream::~FileStream()
{
}

void FileStream::skip(size_t bytes)
{
  // lseek returning -1 can be either an error, or theoretically,
  // a valid new file position. To make sure, we need to
  // clear and check errno.
  errno = 0;

  if (lseek(fd, bytes, SEEK_CUR) == (off_t)-1 && errno)
    throw SystemCallException("lseek", errno, THROW_ARGS);
}

size_t FileStream::size()
{
  struct stat st;

  if (::fstat(fd, &st) != 0)
    throw SystemCallException("fstat", errno, THROW_ARGS);

  return (size_t)st.st_size;
}

} // namespace LOFAR
