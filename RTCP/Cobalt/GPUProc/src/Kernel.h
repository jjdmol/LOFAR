#ifndef GPUPROC_KERNEL_H
#define GPUPROC_KERNEL_H

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "PerformanceCounter.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class Kernel : public cl::Kernel
    {
    public:
      Kernel(const Parset &ps, cl::Program &program, const char *name);

      void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter);

    protected:
      cl::Event event;
      const Parset &ps;
      cl::NDRange globalWorkSize, localWorkSize;
      size_t nrOperations, nrBytesRead, nrBytesWritten;
    };
  }
}
#endif
