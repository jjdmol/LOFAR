#ifndef GPUPROC_BEAMFORMERTRANSPOSEKERNEL_H
#define GPUPROC_BEAMFORMERTRANSPOSEKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class BeamFormerTransposeKernel : public Kernel
    {
    public:
      BeamFormerTransposeKernel(const Parset &ps, cl::Program &program,
                                cl::Buffer &devTransposedData, cl::Buffer &devComplexVoltages);

    };
  }
}
#endif
