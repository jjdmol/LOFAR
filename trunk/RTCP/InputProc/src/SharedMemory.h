#ifndef SHARED_MEMORY
#define SHARED_MEMORY

#include <Common/Exception.h>
#include <Interface/Allocator.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

namespace LOFAR {
namespace RTCP {

class SharedMemoryArena: public FixedArena {
public:
  EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);

  enum Mode {
    CREATE,
    CREATE_EXCL,
    READ,
    READWRITE
  };

  SharedMemoryArena( key_t key, size_t size, Mode mode = CREATE, time_t timeout = 60 );
  ~SharedMemoryArena();

  template <typename T> T* ptr( size_t offset = 0 ) const {
    return reinterpret_cast<T*>(reinterpret_cast<char*>(itsBegin) + offset);
  }

private:
  const key_t key;
  const Mode mode;
  int shmid;
};

template<typename T> class SharedStruct {
public:
  SharedStruct( key_t key, bool create = false, time_t timeout = 60 );

  T &get() {
    return *data.ptr<T>();
  }

  T &get() const {
    return *data.ptr<T>();
  }

private:
  SharedMemoryArena data;

  SharedStruct( const SharedStruct & );
  SharedStruct &operator=( const SharedStruct & );
};


template<typename T> SharedStruct<T>::SharedStruct( key_t key, bool create, time_t timeout )
:
  data(key, sizeof(T), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READWRITE, timeout)
{
}

}
}

#endif

