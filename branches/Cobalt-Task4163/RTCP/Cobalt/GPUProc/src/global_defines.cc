#include "lofar_config.h"
#include <stdlib.h>
#include "CL/cl.hpp"
#include <cstdio>

namespace LOFAR
{
  namespace Cobalt
  {
    bool profiling = false;
    const char *str = getenv("NR_GPUS");
    unsigned nrGPUs = str ? atoi(str) : 1;

        #if defined __linux__

    inline void set_affinity(unsigned device)
    {
#if 0
      static const char mapping[1][12] = {
        0,  1,  2,  3,  8,  9, 10, 11,
      };
#else
      static const char mapping[8][12] = {
        { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
        { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
        { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
        { 0,  1,  2,  3,  4,  5, 12, 13, 14, 15, 16, 17, },
        { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
        { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
        { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
        { 6,  7,  8,  9, 10, 11, 18, 19, 20, 21, 22, 23, },
      };
#endif

      cpu_set_t set;

      CPU_ZERO(&set);

      for (unsigned coreIndex = 0; coreIndex < 12; coreIndex++)
        CPU_SET(mapping[device][coreIndex], &set);

      if (sched_setaffinity(0, sizeof set, &set) < 0)
        perror("sched_setaffinity");
    }

#endif
  }
}
