//# Pool.h: Allocator Pool
//#
//# Copyright (C) 2005
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

#ifndef EXPR_POOL_H
#define EXPR_POOL_H

#include <assert.h>
#include <stdlib.h>

namespace LOFAR
{
namespace BBS
{

// The Pool class is a very fast, stack-based allocator where objects can
// be temporarily stored that would otherwise be deleted and allocated again.
// To delete an object, use deallocate().  To allocate an object, use allocate()
// or allocate(size) (to override the size of the templated class T).
// Note that the object's internal data is clobbered when it is stored in the
// pool.

template <class T> class Pool {
  public:
    inline Pool() : top(0) {
      assert(sizeof(T) >= sizeof(T *));
    }

    inline ~Pool() {
      clear();
    }

    inline bool empty() const {
      return top == 0;
    }

    inline T *allocate(size_t size = sizeof(T)) {
      return empty() ? (T *) malloc(size) : pop();
    }

    inline void deallocate(T *rep) {
      * (T **) rep = top;
      top = rep;
    }

    inline void clear() {
      while (!empty())
	free(pop());
    }

  private:
    inline T *pop() {
      T *rep = top;
      top = * (T **) rep;
      return rep;
    }

    T *top;
};

} // namespace BBS
} // namespace LOFAR

#endif
