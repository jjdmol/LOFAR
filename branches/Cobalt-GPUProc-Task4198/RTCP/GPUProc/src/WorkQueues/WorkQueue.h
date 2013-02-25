#ifndef GPUPROC_WORKQUEUE_H
#define GPUPROC_WORKQUEUE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"
#include "Pipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class WorkQueue
        {
        public:
            WorkQueue(Pipeline &,cl::Context context, unsigned gpuNumber, const Parset	&ps);

            const unsigned	gpu;
            cl::Device		&device;
            cl::CommandQueue	queue;

        protected:
            const Parset	&ps;
        };

    }
}
#endif
