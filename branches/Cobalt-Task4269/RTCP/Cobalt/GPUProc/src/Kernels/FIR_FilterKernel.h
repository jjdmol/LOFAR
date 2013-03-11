#ifndef GPUPROC_FIR_FILTERKERNEL_H
#define GPUPROC_FIR_FILTERKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class FIR_FilterKernel : public Kernel
        {
        public:
            FIR_FilterKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program,
                cl::Buffer &devFilteredData, cl::Buffer &devInputSamples,
                cl::Buffer &devFIRweights);
        };
    }
}

#endif