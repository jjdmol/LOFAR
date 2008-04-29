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

#include <cstdlib>
#include <iostream>


#if defined USE_ZOID_ALLOCATOR
extern "C" {
  void *__zoid_alloc(size_t);
  void __zoid_free(void *);
}
#endif


namespace LOFAR {
namespace CS1 {

#if !defined USE_ZOID_ALLOCATOR
#if defined HAVE_ZOID
FixedArena	   ION_Allocator::arena((void *) 0xA4002400, 0xBFFDC00);
SparseSetAllocator ION_Allocator::allocator(ION_Allocator::arena);
#else
HeapAllocator	   ION_Allocator::allocator;
#endif
#endif


ION_Allocator *ION_Allocator::clone() const
{
  return new ION_Allocator();
}


void *ION_Allocator::allocate(size_t nbytes, unsigned alignment)
{
#if defined USE_ZOID_ALLOCATOR
  void *ptr = __zoid_alloc(nbytes);
#else
  void *ptr = allocator.allocate(nbytes, alignment);
#endif

  std::clog << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") = " << ptr << std::endl;

  if (ptr == 0) {
    std::cerr << "ION_Allocator::allocate(" << nbytes << ", " << alignment << ") : out of large-TLB memory" << std::endl;
    std::exit(1);
  }

  return ptr;
}


void ION_Allocator::deallocate(void *ptr)
{
  std::clog << "ION_Allocator::deallocate(" << ptr << ")" << std::endl;

  if (ptr != 0) {
#if defined USE_ZOID_ALLOCATOR
    __zoid_free(ptr);
#else
    allocator.deallocate(ptr);
#endif
  }
}

} // end namespace CS1
} // end namespace LOFAR
