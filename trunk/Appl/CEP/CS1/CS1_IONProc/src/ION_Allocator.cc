//# ION_Allocator.cc: Class that allocates memory in the large-TLB area of the
//# I/O Node
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <ION_Allocator.h>
#include <CS1_Interface/SparseSet.h>

#include <cstdlib>
#include <iostream>

namespace LOFAR
{

pthread_mutex_t		 ION_Allocator::mutex	 = PTHREAD_MUTEX_INITIALIZER;
SparseSet		 ION_Allocator::freeList = SparseSet().include(0xA4002400, 0xB0000000);
std::map<void *, size_t> ION_Allocator::sizes;

ION_Allocator *ION_Allocator::clone() const
{
  return new ION_Allocator();
}

void *ION_Allocator::allocate(size_t nbytes, size_t alignment)
{
  pthread_mutex_lock(&mutex);

  const std::vector<struct SparseSet::range> &ranges = freeList.getRanges();

  for (std::vector<SparseSet::range>::const_iterator it = ranges.begin(); it != ranges.end(); it ++) {
    size_t begin = (it->begin + alignment - 1) & ~(alignment - 1);

    if (it->end - begin >= nbytes) {
      freeList.exclude(begin, begin + nbytes);
      sizes[(void *) begin] = nbytes;
      std::clog << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") = " << (void *) begin << std::endl;
      pthread_mutex_unlock(&mutex);
      return (void *) begin;
    }
  }

  std::cerr << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") : out of large-TLB memory" << std::endl;
  std::exit(1);
}

void ION_Allocator::deallocate(void *ptr)
{
  std::clog << "ION_Allocator::deallocate(" << ptr << ")" << std::endl;

  if (ptr != 0) {
    pthread_mutex_lock(&mutex);
    std::map<void *, size_t>::iterator index = sizes.find(ptr);
    freeList.include((size_t) ptr, (size_t) ptr + index->second);
    sizes.erase(index);
    pthread_mutex_unlock(&mutex);
  }
}

} // end namespace LOFAR
