#include "SharedMemory.h"
#include <unistd.h>

using namespace LOFAR;

int main() {
  INIT_LOGGER("foo");

  SharedMemory shm( 0x123, 4096, SharedMemory::READ );

  sleep(5);
}
