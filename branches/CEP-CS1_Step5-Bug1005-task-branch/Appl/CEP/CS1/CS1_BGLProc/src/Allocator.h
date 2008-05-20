#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ALLOCATOR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ALLOCATOR_H

#include <CS1_Interface/SparseSet.h>
#include <map>

namespace LOFAR {
namespace CS1 {

class Heap
{
  public:
    Heap(size_t heapSize, int alignment);
    ~Heap();

  private:
    friend class Overlay;
    void	 *start;
    size_t	 size;
};


class Overlay
{
  public:
    Overlay(const Heap &);

    void *allocate(size_t size, int alignment);
    void deallocate(void *ptr);

  private:
    SparseSet<void *>	     freeList;
    std::map<void *, size_t> sizes;
};

} // namespace CS1
} // namespace LOFAR

#endif
