//# Mmap.h: class wrap the mmap(2) system call and friends
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_COMMON_MMAP_H 
#define LOFAR_COMMON_MMAP_H

// \file
// Class wrapping for the mmap and munmap system calls.

#include <sys/mman.h>

namespace LOFAR
{
  // \addtogroup Common

  // This class provides a wrapper around the mmap() and munmap() system calls.
  //
  // The most important restriction is that mappings reside on page granularity.
  // Use sysconf(_SC_PAGE_SIZE) (or friends) (unistd.h) to retrieve the system's
  // page size. Consult the OS man or info pages for more documentation.
  class Mmap
  {
  public:
    Mmap(void *addr/* = NULL*/, size_t length, int prot = PROT_READ,
         int flags = MAP_PRIVATE, int fd = -1, off_t offset = 0);

    ~Mmap();

    void* operator()() { return _ptr; }

  private:
    void *_ptr;
    size_t _len;
  };

} // namespace LOFAR

#endif

