#ifndef GPUPROC_UHEP_TRIGGERKERNEL_H
#define GPUPROC_UHEP_TRIGGERKERNEL_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {


        struct TriggerInfo {
            float	   mean, variance, bestValue;
            unsigned bestApproxIndex;
        };

        class UHEP_TriggerKernel : public Kernel
        {
        public:
            UHEP_TriggerKernel(const Parset &ps, cl::Program &program, 
                cl::Buffer &devTriggerInfo, cl::Buffer &devInvFIRfilteredData);

        };

    }
}
#endif
