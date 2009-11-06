#include <lofar_config.h>

#if defined HAVE_BGP_ION

#include <iostream>
#include <pthread.h>

#include <bpcore/ppc450_inlines.h>

#include <FCNP/fcnp_ion.h>


static char largeBuffer[128 * 1024 * 1024] __attribute__((aligned(16)));


int main(int argc, char **argv)
{
  memset(largeBuffer, 0, sizeof largeBuffer);

  FCNP_ION::init(true);

  for (size_t size = 16; size <= 128 * 1024 * 1024; size <<= 1) {
    FCNP_ION::IONtoCN_ZeroCopy(0, largeBuffer, size);

    unsigned long long start_time = _bgp_GetTimeBase();

    for (unsigned i = 0; i < 15; i ++)
      FCNP_ION::IONtoCN_ZeroCopy(0, largeBuffer, size);

    unsigned long long stop_time = _bgp_GetTimeBase();
    std::cout << "size = " << size << ": ION->CN = " << (15 * 8.0 * size / ((stop_time - start_time) / 850e6)) << std::endl;
  }

  for (size_t size = 16; size <= 128 * 1024 * 1024; size <<= 1) {
    FCNP_ION::CNtoION_ZeroCopy(0, largeBuffer, size);

    unsigned long long start_time = _bgp_GetTimeBase();

    for (unsigned i = 0; i < 15; i ++)
      FCNP_ION::CNtoION_ZeroCopy(0, largeBuffer, size);

    unsigned long long stop_time = _bgp_GetTimeBase();
    std::cout << "size = " << size << ": CN->ION = " << (15 * 8.0 * size / ((stop_time - start_time) / 850e6)) << std::endl;
  }

  FCNP_ION::end();

  return 0;
}

#else

int main()
{
  return 0;
}

#endif // defined HAVE_BGP_ION
