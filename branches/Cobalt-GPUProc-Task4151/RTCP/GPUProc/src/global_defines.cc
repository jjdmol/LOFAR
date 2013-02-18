#include "lofar_config.h"
#include <stdlib.h> 

namespace LOFAR 
{
    namespace RTCP 
    {
        bool	 profiling = true;
        // Select number of GPUs to run on
        const char *str = getenv("NR_GPUS");
        unsigned nrGPUs = str ? atoi(str) : 1;
    }
}
