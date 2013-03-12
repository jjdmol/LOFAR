#ifndef SHARED_MEMORY
#define SHARED_MEMORY

#include <Common/Exception.h>
#include <CoInterface/Allocator.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

namespace LOFAR
{
  namespace RTCP
  {

    /*
     * A memory region manager for shared memory, to be used by
     * allocators defined in CoInterface/Allocator.h
     */
    class SharedMemoryArena : public FixedArena
    {
    public:
      EXCEPTION_CLASS(TimeOutException, LOFAR::Exception);

      enum Mode {
        CREATE,
        CREATE_EXCL,
        READ,
        READWRITE
      };

      /* Create a shared memory region, or attach to an existing one. The timeout
       * specifies how long the constructor will wait for the memory region to
       * appear if mode == READ or mode == READWRITE.
       */
      SharedMemoryArena( key_t key, size_t size, Mode mode = CREATE, time_t timeout = 60 );
      ~SharedMemoryArena();

      template <typename T>
      T* ptr( size_t offset = 0 ) const
      {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(itsBegin) + offset);
      }

      // Return the maximum size for shared-memory buffers,
      // or 0 if the maximum could not be determined.
      static size_t maxSize();

    private:
      const key_t key;
      const Mode mode;
      int shmid;

      // Whether the memory region existed before we tried to create it.
      bool preexisting;

      // Try to open the region indicated by `this->key', and store the pointer
      // in `itsBegin' on success. Used by the constructor.
      //
      // If timeout is false, no errors are silenced. If timeout is true,
      // some errors are ignored to allow subsequent attempts.
      //
      // Returns:
      //   true:  region was opened/created succesfully.
      //   false: region does not exist (and open_flags do not contain CREATE).
      //   throws SystemCallException: a system call failed.
      bool open( int open_flags, int attach_flags, bool timeout);
    };

    /*
     * Provides an interface for any struct stored as a shared memory region.
     */
    template<typename T>
    class SharedStruct
    {
    public:
      SharedStruct( key_t key, bool create = false, time_t timeout = 60 );

      T &get()
      {
        return *data.ptr<T>();
      }

      T &get() const
      {
        return *data.ptr<T>();
      }

    private:
      SharedMemoryArena data;

      SharedStruct( const SharedStruct & );
      SharedStruct &operator=( const SharedStruct & );
    };


    template<typename T>
    SharedStruct<T>::SharedStruct( key_t key, bool create, time_t timeout )
      :
      data(key, sizeof(T), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READWRITE, timeout)
    {
    }

  }
}

#endif

