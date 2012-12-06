#ifndef SHARED_MEMORY
#define SHARED_MEMORY

#include <Common/Exception.h>
#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>
#include <Interface/Allocator.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

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

SharedMemoryArena::SharedMemoryArena( key_t key, size_t size, Mode mode, time_t timeout )
:
  FixedArena(NULL, size),
  key(key),
  mode(mode),
  shmid(-1)
{
  time_t deadline = time(0) + timeout;
  int open_flags = 0, attach_flags = 0;

  switch (mode) {
    case CREATE_EXCL:
      open_flags |= IPC_EXCL;
    case CREATE:
      open_flags |= IPC_CREAT | SHM_NORESERVE | S_IRUSR | S_IWUSR;
      break;

    case READ:
      attach_flags |= SHM_RDONLY;
      break;

    case READWRITE:
    default:
      break;
  }

  // get/create shmid handle
  for(;;) {
    shmid = shmget( key, itsSize, open_flags );

    if (shmid == -1) {
      if (!timeout)
        throw SystemCallException("shmget", errno, THROW_ARGS);

      if (errno != ENOENT && errno != EEXIST)
        throw SystemCallException("shmget", errno, THROW_ARGS);
    } else {
      // attach to segment
      itsBegin = shmat( shmid, NULL, attach_flags );

      if (itsBegin != (void*)-1)
        break; // success!

      if (!timeout)
        throw SystemCallException("shmat", errno, THROW_ARGS);

      if (errno != EINVAL)
        throw SystemCallException("shmat", errno, THROW_ARGS);
    }

    // try again until the deadline

    if (time(0) >= deadline)
      throw TimeOutException("shared memory", THROW_ARGS);

    if (usleep(999999) < 0)
      throw SystemCallException("sleep", errno, THROW_ARGS);
  }

}

SharedMemoryArena::~SharedMemoryArena()
{
  try {
    // detach
    if (shmdt(itsBegin) < 0)
      throw SystemCallException("shmdt", errno, THROW_ARGS);

    // destroy
    if (mode == CREATE || mode == CREATE_EXCL)
      if (shmctl(shmid, IPC_RMID, NULL) < 0)
        throw SystemCallException("shmctl", errno, THROW_ARGS);

  } catch (Exception &ex) {
    LOG_ERROR_STR("Exception in destructor: " << ex);
  }
}

template<typename T> SharedStruct<T>::SharedStruct( key_t key, bool create, time_t timeout )
:
  data(key, sizeof(T), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READWRITE, timeout)
{
}

}
}

#endif

