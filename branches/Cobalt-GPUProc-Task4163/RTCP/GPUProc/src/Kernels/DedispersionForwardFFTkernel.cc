#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "DedispersionForwardFFTkernel.h"
#include "FFT_Kernel.h"
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"


namespace LOFAR
{
    namespace RTCP 
    {        
            DedispersionForwardFFTkernel::DedispersionForwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer)
                :
            FFT_Kernel(context, ps.dedispersionFFTsize(), ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() / ps.dedispersionFFTsize(), true, buffer)
            {
                ASSERT(ps.nrSamplesPerChannel() % ps.dedispersionFFTsize() == 0);
            }
    }
}
