#include "lofar_config.h"    
#include "CL/cl.hpp"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "Pipeline.h"
#include "WorkQueue.h"

namespace LOFAR
{
    namespace  RTCP 
    {      
                WorkQueue::WorkQueue(Pipeline &pipeline, cl::Context context, unsigned queueNumber, const Parset	&ps)
            :
        gpu(queueNumber % nrGPUs),
            device(pipeline.devices[gpu]),
            ps(ps)
        {
#if defined __linux__ && defined USE_B7015
            set_affinity(gpu);
#endif

            queue = cl::CommandQueue(context, device, profiling ? CL_QUEUE_PROFILING_ENABLE : 0);
        }

    }
}
