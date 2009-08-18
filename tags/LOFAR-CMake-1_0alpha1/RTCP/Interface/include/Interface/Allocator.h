#ifndef LOFAR_INTERFACE_ALLOCATOR_H
#define LOFAR_INTERFACE_ALLOCATOR_H

#include <Interface/SparseSet.h>

#include <map>
#include <boost/noncopyable.hpp>

#include <pthread.h>


namespace LOFAR {
namespace RTCP {

// There is a strict separation between a memory allocator and the physical
// memory (arena) that it manages.

class Arena
{
  public:
    virtual	~Arena();

    void	*begin() const { return itsBegin; }
    size_t	size() const { return itsSize; }
  
  protected:
    void	*itsBegin;
    size_t	itsSize;
};


class MallocedArena : public Arena, boost::noncopyable
{
  public:
		MallocedArena(size_t size, size_t alignment);
    virtual	~MallocedArena();
};


class FixedArena : public Arena
{
  public:
    FixedArena(void *begin, size_t size);
};


class Allocator
{
  public:
    virtual			~Allocator();

    virtual void		*allocate(size_t size, size_t alignment = 1) = 0;
    virtual void		deallocate(void *) = 0;
};


class HeapAllocator : public Allocator
{
  public:
    virtual			~HeapAllocator();

    virtual void		*allocate(size_t size, size_t alignment = 1);
    virtual void		deallocate(void *);
};

extern HeapAllocator heapAllocator;


class SparseSetAllocator : public Allocator, boost::noncopyable
{
  public:
				SparseSetAllocator(const Arena &);
				~SparseSetAllocator();

    virtual void		*allocate(size_t size, size_t alignment);
    virtual void		deallocate(void *);

  private:
    pthread_mutex_t		mutex;

    SparseSet<void *>		freeList;
    std::map<void *, size_t>	sizes;
};


} // namespace RTCP
} // namespace LOFAR

#endif
