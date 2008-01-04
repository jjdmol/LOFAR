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

#define USE_ZOID_ALLOCATOR

#if defined USE_ZOID_ALLOCATOR
extern "C" {
  void *__zoid_alloc(size_t);
  void __zoid_free(void *);
}
#endif


namespace LOFAR
{
#if !defined USE_ZOID_ALLOCATOR
pthread_mutex_t		 ION_Allocator::mutex	 = PTHREAD_MUTEX_INITIALIZER;
SparseSet<char *>	 ION_Allocator::freeList = SparseSet<char *>().include((char *) 0xA4002400, (char *) 0xB0000000);
std::map<char *, size_t> ION_Allocator::sizes;
#endif

ION_Allocator *ION_Allocator::clone() const
{
  return new ION_Allocator();
}

void *ION_Allocator::allocate(size_t nbytes, size_t alignment)
{
#if defined USE_ZOID_ALLOCATOR
  void *ptr = __zoid_alloc(nbytes);

  std::clog << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") = " << ptr << std::endl;

  if (ptr != 0)
    return ptr;
#else
  pthread_mutex_lock(&mutex);

  for (SparseSet<char *>::const_iterator it = ranges.getRanges().begin(); it != ranges.getRanges().end(); it ++) {
    char *begin = (char *) (((size_t) it->begin + alignment - 1) & ~(alignment - 1));

    if (it->end - begin >= (ptrdiff_t) nbytes) {
      freeList.exclude(begin, begin + nbytes);
      sizes[begin] = nbytes;
      std::clog << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") = " << (void *) begin << std::endl;
      pthread_mutex_unlock(&mutex);
      return (void *) begin;
    }
  }
#endif

  std::cerr << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") : out of large-TLB memory" << std::endl;
  std::exit(1);
}

void ION_Allocator::deallocate(void *ptr)
{
  std::clog << "ION_Allocator::deallocate(" << ptr << ")" << std::endl;

  if (ptr != 0) {
#if defined USE_ZOID_ALLOCATOR
    __zoid_free(ptr);
#else
    pthread_mutex_lock(&mutex);
    std::map<char *, size_t>::iterator index = sizes.find((char *) ptr);
    freeList.include((char *) ptr, (char *) ptr + index->second);
    sizes.erase(index);
    pthread_mutex_unlock(&mutex);
#endif
  }
}

} // end namespace LOFAR
