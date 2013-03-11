#ifndef GPUPROC_BEAMFORMERKERNEL_H
#define GPUPROC_BEAMFORMERKERNEL_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

#include "global_defines.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class BeamFormerKernel : public Kernel
        {
        public:
            BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages,
                cl::Buffer &devCorrectedData, cl::Buffer &devBeamFormerWeights);
        };


    }
}
#endif