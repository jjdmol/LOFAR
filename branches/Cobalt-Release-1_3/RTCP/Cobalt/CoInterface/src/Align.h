//# Align.h
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

#ifndef LOFAR_INTERFACE_ALIGN_H
#define LOFAR_INTERFACE_ALIGN_H

#include <cstddef>

#include <Common/LofarTypes.h>

namespace LOFAR
{
  namespace Cobalt
  {


    /*
     * Returns true iff n is a power of two. Else, return false.
     * T must be an integral type.
     */
    template <typename T>
    inline static bool powerOfTwo(T n)
    {
      return n > 0 && !(n & (n - 1));
    }


    /*
     * Returns the first power of two greater or equal than n.
     * T must be an integral type.
     * n must be less or equal to the largest representable power of two.
     */
    template <typename T>
    inline static T roundUpToPowerOfTwo(T n)
    {
      n -= 1;
      n |= n >> 1;
      n |= n >> 2;
      n |= n >> 4;
      if (sizeof n > sizeof(uint8))
        n |= n >> 8;
      if (sizeof n > sizeof(uint16))
        n |= n >> 16;
      if (sizeof n > sizeof(uint32))
        n |= (uint64)n >> 32; // quell warning
      n++;
      n += (n == 0);
      return n;
    }


    /*
     * Returns `value' rounded up to `alignment'.
     */
    template <typename T>
    inline static T align(T value, size_t alignment)
    {
#if defined __GNUC__
      if (__builtin_constant_p(alignment) && powerOfTwo(alignment))
        return (value + alignment - 1) & ~(alignment - 1);
      else
#endif
      return (value + alignment - 1) / alignment * alignment;
    }


    /*
     * Returns `value' rounded up to `alignment', in bytes.
     */
    template <typename T>
    inline static T *align(T *value, size_t alignment)
    {
      return reinterpret_cast<T *>(align(reinterpret_cast<size_t>(value), alignment));
    }


    /*
     * Returns true if `value' is aligned to `alignment'.
     */
    template <typename T>
    inline static bool aligned(T value, size_t alignment)
    {
      return value % alignment == 0;
    }


    /*
     * Returns true if `value' is aligned to `alignment', in bytes.
     */
    template <typename T>
    inline static bool aligned(T *value, size_t alignment)
    {
      return reinterpret_cast<size_t>(value) % alignment == 0;
    }


  } // namespace Cobalt
} // namespace LOFAR

#endif

