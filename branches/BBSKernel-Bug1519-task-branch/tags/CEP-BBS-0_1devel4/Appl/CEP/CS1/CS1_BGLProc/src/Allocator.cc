#include <lofar_config.h>

#include <Allocator.h>

#include <malloc.h>


namespace LOFAR {
namespace CS1 {

Heap::Heap(size_t heapSize, int alignment)
{
  size = heapSize;

  if (posix_memalign(&start, alignment, heapSize) != 0) {
    std::cerr << "could not allocate heap" << std::endl;
    exit(1);
  }
}


Heap::~Heap()
{
  free(start);
}


Overlay::Overlay(const Heap &heap)
{

  freeList.include(heap.start, (void *) ((char *) heap.start + heap.size));
}


void *Overlay::allocate(size_t size, int alignment)
{
  for (SparseSet<void *>::const_iterator it = freeList.getRanges().begin(); it != freeList.getRanges().end(); it ++) {
    void *begin = (void *) (((size_t) it->begin + alignment - 1) & ~(alignment - 1));

    if ((char *) it->end - (char *) begin >= (ptrdiff_t) size) {
      freeList.exclude(begin, (void *) ((char *) begin + size));
      sizes[begin] = size;
      return begin;
    }
  }

  std::cerr << "could not allocate memory from heap" << std::endl;
  std::exit(1);
}


void Overlay::deallocate(void *ptr)
{
  std::map<void *, size_t>::iterator index = sizes.find(ptr);
  freeList.include(ptr, (void *) ((char *) ptr + index->second));
  sizes.erase(index);
}

} // namespace CS1
} // namespace LOFAR
