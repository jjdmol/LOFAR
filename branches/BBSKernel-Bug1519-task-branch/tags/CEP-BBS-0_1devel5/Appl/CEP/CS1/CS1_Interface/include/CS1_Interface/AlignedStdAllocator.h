#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_ALIGNED_ALLOCATOR_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_ALIGNED_ALLOCATOR_H

#include <memory>
#include <stdexcept>

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
#include <cstdlib>
#else
#include <malloc.h>
#endif


namespace LOFAR {
namespace CS1 {

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
	throw std::bad_alloc();
#else
      if ((ptr = memalign(ALIGNMENT, size * sizeof(T))) == 0)
	throw std::bad_alloc();
#endif

      return static_cast<pointer>(ptr);
    }

    void deallocate(pointer ptr, size_type /*size*/)
    {
      free(ptr);
    }
};

} // namespace CS1
} // namespace LOFAR

#endif
