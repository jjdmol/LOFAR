#ifndef GPUPROC_INCOHERENTSTOKESKERNEL_H
#define GPUPROC_INCOHERENTSTOKESKERNEL_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {

        class IncoherentStokesKernel : public Kernel
        {
        public:
            IncoherentStokesKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, 
                cl::Buffer &devIncoherentStokes, cl::Buffer &devInputSamples);             
        };

    }
}
#endif
