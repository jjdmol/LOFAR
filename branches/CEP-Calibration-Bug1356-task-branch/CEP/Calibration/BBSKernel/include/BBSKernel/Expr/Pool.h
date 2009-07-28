#ifndef EXPR_POOL_H
#define EXPR_POOL_H

#include <assert.h>
#include <stdlib.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

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

// @}

} // namespace BBS
} // namespace LOFAR

#endif
