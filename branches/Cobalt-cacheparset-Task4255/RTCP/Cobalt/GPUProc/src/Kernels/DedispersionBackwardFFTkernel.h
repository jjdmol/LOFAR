#ifndef GPUPROC_DEDISPERSIONBACKWARDFFTKERNEL_H
#define GPUPROC_DEDISPERSIONBACKWARDFFTKERNEL_H

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "FFT_Kernel.h"

namespace LOFAR
{
  namespace RTCP
  {
    class DedispersionBackwardFFTkernel : public FFT_Kernel
    {
    public:
      DedispersionBackwardFFTkernel(const Parset &ps, cl::Context &context, cl::Buffer &buffer);

    };
  }

}
#endif
