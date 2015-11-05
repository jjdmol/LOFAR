#include <lofar_config.h>

#include <Interface/Align.h>
#include <Interface/Allocator.h>
#include <Interface/Exceptions.h>
#include <Common/NewHandler.h>

#include <malloc.h>


namespace LOFAR {
namespace RTCP {


Arena::~Arena()
{
}


MallocedArena::MallocedArena(size_t size, size_t alignment)
{
  itsBegin = heapAllocator.allocate(size, alignment);
  itsSize = size;
}


MallocedArena::~MallocedArena()
{
  heapAllocator.deallocate(itsBegin);
}


FixedArena::FixedArena(void *begin, size_t size)
{
  itsBegin = begin;
  itsSize  = size;
}


Allocator::~Allocator()
{
}


HeapAllocator::~HeapAllocator()
{
}


void *HeapAllocator::allocate(size_t size, size_t alignment)
{
  void *ptr;

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
  if (posix_memalign(&ptr, alignment, size) != 0)
    THROW(BadAllocException,"HeapAllocator could not allocate " << size << " bytes");
#else
  if ((ptr = memalign(alignment, size)) == 0)
    THROW(BadAllocException,"HeapAllocator could not allocate " << size << " bytes");
#endif

  return ptr;
}


void HeapAllocator::deallocate(void *ptr)
{
  free(ptr);
}


HeapAllocator heapAllocator;


SparseSetAllocator::SparseSetAllocator(const Arena &arena)
{
  freeList.include(arena.begin(), (void *) ((char *) arena.begin() + arena.size()));
}


void *SparseSetAllocator::allocate(size_t size, size_t alignment)
{
  ScopedLock sl(mutex);

  for (SparseSet<void *>::const_iterator it = freeList.getRanges().begin(); it != freeList.getRanges().end(); it ++) {
    void *begin = align(it->begin, alignment);

    if ((char *) it->end - (char *) begin >= (ptrdiff_t) size) {
      freeList.exclude(begin, (void *) ((char *) begin + size));
      sizes[begin] = size;

      return begin;
    }
  }

  THROW(InterfaceException,"SparseSetAllocator could not allocate " << size << " bytes");
}


void SparseSetAllocator::deallocate(void *ptr)
{
  if (ptr != 0) {
    ScopedLock sl(mutex);

    std::map<void *, size_t>::iterator index = sizes.find(ptr);
    freeList.include(ptr, (void *) ((char *) ptr + index->second));
    sizes.erase(index);
  }
}


} // namespace RTCP
} // namespace LOFAR
