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

#define _FILE_OFFSET_BITS 64
#include <Common/SystemCallException.h>
#include <Stream/FileStream.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {

FileStream::FileStream(const std::string &name)
{
  if ((fd = ::open(name.c_str(), O_RDONLY)) < 0)
    THROW_SYSCALL(std::string("open ") + name);
}


FileStream::FileStream(const std::string &name, int mode)
{
  if ((fd = ::open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode)) < 0)
    THROW_SYSCALL(std::string("open ") + name);
}


FileStream::FileStream(const std::string &name, int flags, int mode)
{
  if ((fd = ::open(name.c_str(), flags, mode)) < 0) 
    THROW_SYSCALL(std::string("open ") + name);
}

FileStream::~FileStream()
{
}

void FileStream::skip(size_t bytes)
{
  if (::lseek(fd, bytes, SEEK_CUR) < 0)
    THROW_SYSCALL("lseek");
}

size_t FileStream::size()
{
  struct stat st;

  if (::fstat(fd, &st) != 0)
    THROW_SYSCALL("fstat");

  return (size_t)st.st_size;
}

} // namespace LOFAR
