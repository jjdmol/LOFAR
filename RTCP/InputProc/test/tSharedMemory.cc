#include <lofar_config.h>
#include "SharedMemory.h"
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace RTCP;

int main( int, char **argv ) {
  INIT_LOGGER( argv[0] );

  /* Create a shared memory region */
  {
    LOG_INFO("Create shared memory region");

    SharedMemoryArena m( 0x12345678, 1024, SharedMemoryArena::CREATE, 0 );
  }
}
