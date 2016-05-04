//# Mmap.cc: class to wrap the mmap and munmap system calls
//# Copyright (C) 2013, 2015  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Mmap.h>

#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

  Mmap::Mmap(void *addr, size_t length, int prot, int flags, int fd,
             off_t offset) :
    _len(length)
  {
    if (flags & MAP_ANON) {
      fd = -1; // portability
    }
    _ptr = ::mmap(addr, length, prot, flags, fd, offset);
    if (_ptr == MAP_FAILED) {
       THROW_SYSCALL("mmap");
    }
  }

  Mmap::~Mmap()
  {
    if (::munmap(_ptr, _len) == -1) {
      LOG_WARN("munmap() failed"); // do not throw in a destructor
    }
  }

} // namespace LOFAR

