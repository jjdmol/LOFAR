#ifndef GPUPROC_UHEP_INVFIR_KERNEL_H
#define GPUPROC_UHEP_INVFIR_KERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class UHEP_InvFIR_Kernel : public Kernel
        {
        public:
            UHEP_InvFIR_Kernel(const Parset &ps, cl::CommandQueue &queue, 
                cl::Program &program, cl::Buffer &devInvFIRfilteredData, 
                cl::Buffer &devFFTedData, cl::Buffer &devInvFIRfilterWeights);
        };

    }
}
#endif
