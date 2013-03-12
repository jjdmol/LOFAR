#ifndef GPUPROC_UHEP_TRANSPOSEKERNEL_H
#define GPUPROC_UHEP_TRANSPOSEKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"
namespace LOFAR
{
  namespace Cobalt
  {
    class UHEP_TransposeKernel : public Kernel
    {
    public:
      UHEP_TransposeKernel(const Parset &ps, cl::Program &program,
                           cl::Buffer &devFFTedData, cl::Buffer &devComplexVoltages, cl::Buffer &devReverseSubbandMapping);
    };

  }
}
#endif
