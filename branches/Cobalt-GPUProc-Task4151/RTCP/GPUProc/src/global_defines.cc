#include "lofar_config.h"
#include <stdlib.h> 
#include "CL/cl.hpp"

namespace LOFAR 
{
    namespace RTCP 
    {
        bool	 profiling = true;
        const char *str = getenv("NR_GPUS");
        unsigned nrGPUs = str ? atoi(str) : 1;
    }
}
