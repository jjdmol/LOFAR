//# AlignedStdAllocator.h
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_INTERFACE_ALIGNED_ALLOCATOR_H
#define LOFAR_INTERFACE_ALIGNED_ALLOCATOR_H

#include <memory>

#include <CoInterface/Allocator.h>

namespace LOFAR
{
  namespace Cobalt
  {

    template <typename T, size_t ALIGNMENT>
    class AlignedStdAllocator : public std::allocator<T>
    {
      // NOTE: An std::allocator cannot hold *any* state because they're
      // constructed and destructed at will by the STL. The STL does not
      // even guarantee that the same allocator object will allocate and
      // deallocate a certain pointer.
    public:
      typedef typename std::allocator<T>::size_type size_type;
      typedef typename std::allocator<T>::pointer pointer;
      typedef typename std::allocator<T>::const_pointer const_pointer;

      template <class U>
      struct rebind
      {
        typedef AlignedStdAllocator<U, ALIGNMENT> other;
      };

      pointer allocate(size_type size, const_pointer /*hint*/ = 0)
      {
        return static_cast<pointer>(heapAllocator.allocate(size * sizeof(T), ALIGNMENT));
      }

      void deallocate(pointer ptr, size_type /*size*/)
      {
        heapAllocator.deallocate(ptr);
      }
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

