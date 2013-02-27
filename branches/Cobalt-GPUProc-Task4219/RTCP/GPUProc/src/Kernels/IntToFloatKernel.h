#ifndef GPUPROC_INTTOFLOATKERNEL_H
#define GPUPROC_INTTOFLOATKERNEL_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {
                class IntToFloatKernel : public Kernel
        {
        public:
            IntToFloatKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program,
                cl::Buffer &devFilteredData, cl::Buffer &devInputSamples);
        };
    }

}
#endif

