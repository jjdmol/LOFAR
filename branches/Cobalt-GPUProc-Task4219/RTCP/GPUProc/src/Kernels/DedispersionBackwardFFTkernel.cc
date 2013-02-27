#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "DedispersionBackwardFFTkernel.h"
#include "FFT_Kernel.h"
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"


namespace LOFAR
{
    namespace RTCP 
    {    
            DedispersionBackwardFFTkernel::DedispersionBackwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
                :
            FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), false, buffer)
            {
                ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
            }
    }

}