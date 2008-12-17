#ifndef LOFAR_INTERFACE_ALIGNED_ALLOCATOR_H
#define LOFAR_INTERFACE_ALIGNED_ALLOCATOR_H

#include <memory>
#include <stdexcept>

#include <Interface/Exceptions.h>

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
#include <cstdlib>
#else
#include <malloc.h>
#endif


namespace LOFAR {
namespace RTCP {

template <typename T, size_t ALIGNMENT> class AlignedStdAllocator : public std::allocator<T>
{
  public:
    typedef typename std::allocator<T>::size_type size_type;
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::const_pointer const_pointer;

    template <class U> struct rebind
    {
      typedef AlignedStdAllocator<U, ALIGNMENT> other;
    };

    pointer allocate(size_type size, const_pointer /*hint*/ = 0)
    {
      void *ptr;
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
      if (posix_memalign(&ptr, ALIGNMENT, size * sizeof(T)) != 0)
	THROW(AssertError, "Out of memory");
#else
      if ((ptr = memalign(ALIGNMENT, size * sizeof(T))) == 0)
	THROW(AssertError, "Out of memory");
#endif

      return static_cast<pointer>(ptr);
    }

    void deallocate(pointer ptr, size_type /*size*/)
    {
      free(ptr);
    }
};

} // namespace RTCP
} // namespace LOFAR

#endif
