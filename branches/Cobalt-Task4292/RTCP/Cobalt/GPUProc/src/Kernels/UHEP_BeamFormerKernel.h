#ifndef GPUPROC_UHEP_BEAMFORMERKERNEL_H
#define GPUPROC_UHEP_BEAMFORMERKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class UHEP_BeamFormerKernel : public Kernel
    {
    public:
      UHEP_BeamFormerKernel(const Parset &ps, cl::Program &program,
                            cl::Buffer &devComplexVoltages, cl::Buffer &devInputSamples, cl::Buffer &devBeamFormerWeights);
    };
  }
}
#endif
