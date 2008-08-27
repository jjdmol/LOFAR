#include <lofar_config.h>

#include <CS1_Interface/Align.h>
#include <CS1_Interface/Allocator.h>

#include <malloc.h>


namespace LOFAR {
namespace CS1 {


Arena::~Arena()
{
}


MallocedArena::MallocedArena(size_t size, unsigned alignment)
{
  itsSize = size;

  if (posix_memalign(&itsBegin, alignment, size) != 0) {
    std::cerr << "could not allocate data" << std::endl;
    exit(1);
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


SparseSetAllocator::SparseSetAllocator(const Arena &arena)
{

  freeList.include(arena.begin(), (void *) ((char *) arena.begin() + arena.size()));

#if defined HAVE_THREADS
  pthread_mutex_init(&mutex, 0);
#endif
}


SparseSetAllocator::~SparseSetAllocator()
{
#if defined HAVE_THREADS
  pthread_mutex_destroy(&mutex);
#endif
}


void *SparseSetAllocator::allocate(size_t size, unsigned alignment)
{
#if defined HAVE_THREADS
  pthreads_mutex_lock(&mutex);
#endif

  for (SparseSet<void *>::const_iterator it = freeList.getRanges().begin(); it != freeList.getRanges().end(); it ++) {
    void *begin = align(it->begin, alignment);

    if ((char *) it->end - (char *) begin >= (ptrdiff_t) size) {
      freeList.exclude(begin, (void *) ((char *) begin + size));
      sizes[begin] = size;

#if defined HAVE_THREADS
      pthreads_mutex_unlock(&mutex);
#endif

      return begin;
    }
  }

  std::cerr << "could not allocate memory from arena" << std::endl;
  std::exit(1);
}


void SparseSetAllocator::deallocate(void *ptr)
{
  if (ptr != 0) {
#if defined HAVE_THREADS
    pthreads_mutex_lock(&mutex);
#endif

    std::map<void *, size_t>::iterator index = sizes.find(ptr);
    freeList.include(ptr, (void *) ((char *) ptr + index->second));
    sizes.erase(index);

#if defined HAVE_THREADS
    pthreads_mutex_unlock(&mutex);
#endif
  }
}


SparseSetAllocator *SparseSetAllocator::clone() const
{
  // What is clone() supposed to do ???  Does CEPFrame use it ???
  return const_cast<SparseSetAllocator *>(this);
}

} // namespace CS1
} // namespace LOFAR
