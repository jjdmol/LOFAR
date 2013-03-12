#ifndef GPUPROC_COHERENTSTOKESEKERNEL_H
#define GPUPROC_COHERENTSTOKESEKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"
namespace LOFAR
{
  namespace RTCP
  {

    class CoherentStokesKernel : public Kernel
    {
    public:
      CoherentStokesKernel(const Parset &ps, cl::Program &program,
                           cl::Buffer &devStokesData, cl::Buffer &devComplexVoltages);

    };
  }
}
#endif
