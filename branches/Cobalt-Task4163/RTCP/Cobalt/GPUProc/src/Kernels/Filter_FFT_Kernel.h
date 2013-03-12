#ifndef GPUPROC_FILTER_FFT_KERNEL_H
#define GPUPROC_FILTER_FFT_KERNEL_H

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "FFT_Kernel.h"

namespace LOFAR
{
  namespace RTCP
  {
    class Filter_FFT_Kernel : public FFT_Kernel
    {
    public:
      Filter_FFT_Kernel(const Parset &ps, cl::Context &context,
                        cl::Buffer &devFilteredData);

    };

  }
}
#endif