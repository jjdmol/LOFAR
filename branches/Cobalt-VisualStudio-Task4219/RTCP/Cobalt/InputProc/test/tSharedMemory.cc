#include <lofar_config.h>
#include "Buffer/SharedMemory.h"
#include <Common/LofarLogger.h>
#include <Common/Thread/Thread.h>
#include <unistd.h>

using namespace LOFAR;
using namespace RTCP;

Semaphore semaphore;

class A {
public:
  void creator() {
    sleep(1);

    SharedMemoryArena m( 0x12345678, 1024, SharedMemoryArena::CREATE, 0 );

    LOG_INFO("Memory area created");

    // wait for done
    semaphore.down();
  }

  void reader() {
    LOG_INFO("Waiting for memory area");

    SharedMemoryArena m( 0x12345678, 1024, SharedMemoryArena::READ, 2 );

    LOG_INFO("Memory area attached");

    // signal done
    semaphore.up();
  }
};

int main() {
  INIT_LOGGER( "tSharedMemory" );

  /* Create a shared memory region */
  {
    LOG_INFO("Create shared memory region");

    SharedMemoryArena m( 0x12345678, 1024, SharedMemoryArena::CREATE, 0 );
  }

  /* Create a shared memory region and access it */
  {
    LOG_INFO("Create shared memory region and access it");

    SharedMemoryArena x( 0x12345678, 1024, SharedMemoryArena::CREATE, 0 );

    SharedMemoryArena y( 0x12345678, 1024, SharedMemoryArena::READ, 0 );
  }

  /* Access a non-existing shared memory region */
  {
    LOG_INFO("Access a non-existing shared memory region");

    bool caught_exception = false;

    try {
      SharedMemoryArena y( 0x12345678, 1024, SharedMemoryArena::READ, 0 );
    } catch(SystemCallException &e) {
      caught_exception = true;
    }

    ASSERT(caught_exception);
  }

  /* Access a non-existing shared memory region, with timeout */
  {
    LOG_INFO("Access a non-existing shared memory region");

    bool caught_exception = false;

    try {
      SharedMemoryArena y( 0x12345678, 1024, SharedMemoryArena::READ, 1 );
    } catch(SharedMemoryArena::TimeOutException &e) {
      caught_exception = true;
    }

    ASSERT(caught_exception);
  }

#ifdef USE_THREADS
  LOG_INFO("Debugging concurrent access");

  {
    /* Start reader before creator */
    A obj;

    // delayed creation of memory region
    Thread creator(&obj, &A::creator);

    // wait for access
    obj.reader();
  }
#endif

  /* Check whether memory access works as expected */
  {
    LOG_INFO("Checking memory access through SharedStruct");

    SharedStruct<int> writer( 0x12345678, true, 0 );

    SharedStruct<int> reader( 0x12345678, false, 0 );

    writer.get() = 42;
    ASSERT( reader.get() == 42 );
  }

  return 0;
}
