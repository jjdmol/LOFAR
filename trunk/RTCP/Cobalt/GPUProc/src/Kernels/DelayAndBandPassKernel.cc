#include "lofar_config.h"    

#include "Kernel.h"
#include "DelayAndBandPassKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenCL_FFT/clFFT.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {
        DelayAndBandPassKernel::DelayAndBandPassKernel(const Parset &ps, cl::Program &program, 
            cl::Buffer &devCorrectedData, cl::Buffer &devFilteredData,
            cl::Buffer &devDelaysAtBegin, cl::Buffer &devDelaysAfterEnd,
            cl::Buffer &devPhaseOffsets, cl::Buffer &devBandPassCorrectionWeights)
            :
        Kernel(ps, program, "applyDelaysAndCorrectBandPass")
        {
            ASSERT(ps.nrChannelsPerSubband() % 16 == 0 || ps.nrChannelsPerSubband() == 1);
            ASSERT(ps.nrSamplesPerChannel() % 16 == 0);

            setArg(0, devCorrectedData);
            setArg(1, devFilteredData);
            setArg(4, devDelaysAtBegin);
            setArg(5, devDelaysAfterEnd);
            setArg(6, devPhaseOffsets);
            setArg(7, devBandPassCorrectionWeights);

            globalWorkSize = cl::NDRange(256, ps.nrChannelsPerSubband() == 1 ? 1 : ps.nrChannelsPerSubband() / 16, ps.nrStations());
            localWorkSize  = cl::NDRange(256, 1, 1);

            size_t nrSamples = ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
            nrOperations = nrSamples * 12;
            nrBytesRead = nrBytesWritten = nrSamples * sizeof(std::complex<float>);
        }

        void DelayAndBandPassKernel::enqueue(cl::CommandQueue &queue, PerformanceCounter &counter, unsigned subband)
        {
            setArg(2, (float) ps.subbandToFrequencyMapping()[subband]);
            setArg(3, 0); // beam
            Kernel::enqueue(queue, counter);
        }


    }
}
