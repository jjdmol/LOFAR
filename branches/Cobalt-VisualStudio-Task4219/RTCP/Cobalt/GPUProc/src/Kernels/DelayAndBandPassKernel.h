#ifndef GPUPROC_DELAYANDBANDPASSKERNEL_H
#define GPUPROC_DELAYANDBANDPASSKERNEL_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

#include "Kernel.h"

namespace LOFAR
{
    namespace RTCP 
    {

        class DelayAndBandPassKernel : public Kernel
        {
        public:
            DelayAndBandPassKernel(const Parset &ps, cl::Program &program, cl::Buffer &devCorrectedData, cl::Buffer &devFilteredData, cl::Buffer &devDelaysAtBegin, cl::Buffer &devDelaysAfterEnd, cl::Buffer &devPhaseOffsets, cl::Buffer &devBandPassCorrectionWeights);

            void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, unsigned subband);
        };
    }
}
#endif
