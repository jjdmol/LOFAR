#ifndef GPUPROC_CORRELATORWORKQUEUE_H
#define GPUPROC_CORRELATORWORKQUEUE_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

#include "global_defines.h"

#include "WorkQueue.h"

#include "Kernels/FIR_FilterKernel.h"
#include "Kernels/Filter_FFT_Kernel.h"
#include "Kernels/DelayAndBandPassKernel.h"
#include "Kernels/CorrelatorKernel.h"

#include "Pipelines/CorrelatorPipelinePrograms.h"
#include "Pipelines/CorrelatorPipelineCounters.h"

#include "FilterBank.h"

namespace LOFAR
{
    namespace RTCP 
    {
        class CorrelatorWorkQueue : public WorkQueue
        {
        public:
            CorrelatorWorkQueue(const Parset	&parset,cl::Context context, cl::Device		&device, unsigned queueNumber,
              CorrelatorPipelinePrograms &programs, CorrelatorPipelineCounters &counters,
              FilterBank &filterBank);

            void doWork();

#if defined USE_TEST_DATA
            void setTestPattern();
            void printTestOutput();
#endif

            //private:
            void doSubband(unsigned block, unsigned subband);
            //void receiveSubbandSamples(unsigned block, unsigned subband);
            //void sendSubbandVisibilites(unsigned block, unsigned subband);
            CorrelatorPipelineCounters &counters;
            cl::Buffer		devFIRweights;
            cl::Buffer		devBufferA, devBufferB;
            MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;
            MultiArraySharedBuffer<float, 3> delaysAtBegin, delaysAfterEnd;
            MultiArraySharedBuffer<float, 2> phaseOffsets;
            MultiArraySharedBuffer<char, 4> inputSamples;

            cl::Buffer		devFilteredData;
            cl::Buffer		devCorrectedData;

            MultiArraySharedBuffer<std::complex<float>, 4> visibilities;

            FIR_FilterKernel		firFilterKernel;
            Filter_FFT_Kernel		fftKernel;
            DelayAndBandPassKernel	delayAndBandPassKernel;
#if defined USE_NEW_CORRELATOR
            CorrelateTriangleKernel	correlateTriangleKernel;
            CorrelateRectangleKernel	correlateRectangleKernel;
#else
            CorrelatorKernel		correlatorKernel;
#endif
        };

    }
}
#endif
