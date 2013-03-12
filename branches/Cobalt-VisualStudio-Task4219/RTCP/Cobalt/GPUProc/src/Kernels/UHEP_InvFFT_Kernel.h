#ifndef GPUPROC_UHEP_INVFFT_KERNEL_H
#define GPUPROC_UHEP_INVFFT_KERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class UHEP_InvFFT_Kernel : public Kernel
        {
        public:
            UHEP_InvFFT_Kernel(const Parset &ps, cl::Program &program, cl::Buffer &devFFTedData);
                
        };
    }
}
#endif