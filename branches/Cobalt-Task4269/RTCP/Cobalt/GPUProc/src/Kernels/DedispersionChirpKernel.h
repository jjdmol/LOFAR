#ifndef GPUPROC_DEDISPERSIONCHIRPKERNEL_H
#define GPUPROC_DEDISPERSIONCHIRPKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
  namespace RTCP
  {

    class DedispersionChirpKernel : public Kernel
    {
    public:
      DedispersionChirpKernel(const Parset &ps, cl::Program &program,
                              cl::CommandQueue &queue, cl::Buffer &buffer, cl::Buffer &DMs);

      void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, double subbandFrequency);

    };

  }
}
#endif