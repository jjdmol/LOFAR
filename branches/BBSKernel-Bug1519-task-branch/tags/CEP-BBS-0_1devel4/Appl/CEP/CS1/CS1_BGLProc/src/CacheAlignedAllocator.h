#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CACHE_ALIGNED_ALLOCATOR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CACHE_ALIGNED_ALLOCATOR_H

#include <malloc.h>
#include <memory>


#if defined HAVE_BGL
#define CACHE_LINE_SIZE	32
#define CACHE_ALIGNED	__attribute__ ((aligned(CACHE_LINE_SIZE)))
#else
#define CACHE_LINE_SIZE	16
#define CACHE_ALIGNED
#endif


namespace LOFAR {
namespace CS1 {

template <typename T> class CacheAlignedAllocator : public std::allocator<T>
{
  public:
    typedef typename std::allocator<T>::size_type size_type;
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::const_pointer const_pointer;

    pointer allocate(size_type size, const_pointer /*hint*/ = 0)
    {
      return static_cast<pointer>(memalign(CACHE_LINE_SIZE, size * sizeof(T)));
    }

    void deallocate(pointer ptr, size_type /*size*/)
    {
      free(ptr);
    }
};

} // namespace CS1
} // namespace LOFAR

#endif
