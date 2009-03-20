#include <lofar_config.h>

#include <Interface/Align.h>
#include <Interface/Allocator.h>
#include <Interface/Exceptions.h>

#include <malloc.h>


namespace LOFAR {
namespace RTCP {


Arena::~Arena()
{
}


MallocedArena::MallocedArena(size_t size, size_t alignment)
{
  itsSize = size;

  if (posix_memalign(&itsBegin, alignment, size) != 0) {
    THROW(InterfaceException,"could not allocate data");
  }
}


MallocedArena::~MallocedArena()
{
  free(itsBegin);
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
    throw std::bad_alloc();
#else
  if ((ptr = memalign(alignment, size)) == 0)
    throw std::bad_alloc();
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
  pthread_mutex_init(&mutex, 0);
}


SparseSetAllocator::~SparseSetAllocator()
{
  pthread_mutex_destroy(&mutex);
}


void *SparseSetAllocator::allocate(size_t size, size_t alignment)
{
  pthread_mutex_lock(&mutex);

  for (SparseSet<void *>::const_iterator it = freeList.getRanges().begin(); it != freeList.getRanges().end(); it ++) {
    void *begin = align(it->begin, alignment);

    if ((char *) it->end - (char *) begin >= (ptrdiff_t) size) {
      freeList.exclude(begin, (void *) ((char *) begin + size));
      sizes[begin] = size;

      pthread_mutex_unlock(&mutex);

      return begin;
    }
  }

  pthread_mutex_unlock(&mutex);

  THROW(InterfaceException,"could not allocate data");
}


void SparseSetAllocator::deallocate(void *ptr)
{
  if (ptr != 0) {
    pthread_mutex_lock(&mutex);

    std::map<void *, size_t>::iterator index = sizes.find(ptr);
    freeList.include(ptr, (void *) ((char *) ptr + index->second));
    sizes.erase(index);

    pthread_mutex_unlock(&mutex);
  }
}

} // namespace RTCP
} // namespace LOFAR
