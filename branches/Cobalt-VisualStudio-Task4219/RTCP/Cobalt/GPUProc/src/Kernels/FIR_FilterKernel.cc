#include "lofar_config.h"    

#include "Kernel.h"
#include "FIR_FilterKernel.h"

#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenCL_FFT/clFFT.h"
#include <complex>


namespace LOFAR
{
    namespace RTCP 
    {
        FIR_FilterKernel::FIR_FilterKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devFilteredData, cl::Buffer &devInputSamples, cl::Buffer &devFIRweights)
            :
        Kernel(ps, program, "FIR_filter")
        {
            setArg(0, devFilteredData);
            setArg(1, devInputSamples);
            setArg(2, devFIRweights);

            size_t maxNrThreads;
            getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
            unsigned totalNrThreads = ps.nrChannelsPerSubband() * NR_POLARIZATIONS * 2;
            unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
            globalWorkSize = cl::NDRange(totalNrThreads, ps.nrStations());
            localWorkSize  = cl::NDRange(totalNrThreads / nrPasses, 1);

            size_t nrSamples = (size_t) ps.nrStations() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
            nrOperations   = nrSamples * ps.nrSamplesPerChannel() * NR_TAPS * 2 * 2;
            nrBytesRead    = nrSamples * (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrBytesPerComplexSample();
            nrBytesWritten = nrSamples * ps.nrSamplesPerChannel() * sizeof(std::complex<float>);
        }

    }
}

