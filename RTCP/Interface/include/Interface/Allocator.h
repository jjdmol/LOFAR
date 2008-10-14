#ifndef LOFAR_INTERFACE_ALLOCATOR_H
#define LOFAR_INTERFACE_ALLOCATOR_H

#include <Common/Allocator.h>
#include <Interface/SparseSet.h>

#include <map>


namespace LOFAR {
namespace RTCP {

// There is a strict separation between a memory allocator and the physical
// memory (arena) that it manages.

class Arena
{
  public:
    virtual	~Arena();

    void	*begin() const;
    size_t	size() const;
  
  protected:
    void	*itsBegin;
    size_t	itsSize;
};

class MallocedArena : public Arena
{
  public:
		MallocedArena(size_t size, unsigned alignment);
    virtual	~MallocedArena();
};

class FixedArena : public Arena
{
  public:
    FixedArena(void *begin, size_t size);
};


class SparseSetAllocator : public Allocator
{
  public:
				SparseSetAllocator(const Arena &);
				~SparseSetAllocator();

    virtual void		*allocate(size_t size, unsigned alignment);
    virtual void		deallocate(void *);

    virtual SparseSetAllocator	*clone() const;

  private:
#if defined HAVE_THREADS
    pthread_mutex_t		mutex;
#endif

    SparseSet<void *>		freeList;
    std::map<void *, size_t>	sizes;
};


inline void *Arena::begin() const
{
  return itsBegin;
}

inline size_t Arena::size() const
{
  return itsSize;
}

} // namespace RTCP
} // namespace LOFAR

#endif
