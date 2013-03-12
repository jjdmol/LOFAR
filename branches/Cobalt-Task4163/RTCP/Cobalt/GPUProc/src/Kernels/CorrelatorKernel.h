#ifndef GPUPROC_CORRELATORKERNEL_H
#define GPUPROC_CORRELATORKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
  namespace RTCP
  {
#if !defined USE_NEW_CORRELATOR

    class CorrelatorKernel : public Kernel
    {
    public:
      CorrelatorKernel(const Parset &ps, cl::CommandQueue &queue,
                       cl::Program &program, cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData);
    };

#else

    class CorrelatorKernel : public Kernel
    {
    public:
      CorrelatorKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program,
                       cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData);

    };

    class CorrelateRectangleKernel : public Kernel
    {
    public:
      CorrelateRectangleKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program,
                               cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData);
    };

    class CorrelateTriangleKernel : public Kernel
    {
    public:
      CorrelateTriangleKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program,
                              cl::Buffer &devVisibilities, cl::Buffer &devCorrectedData);
    };

#endif
  }
}
#endif
