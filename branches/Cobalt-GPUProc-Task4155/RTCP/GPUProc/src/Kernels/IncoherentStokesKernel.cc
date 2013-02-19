#include "lofar_config.h"    

#include "Kernel.h"
#include "IncoherentStokesKernel.h"

#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {        
        IncoherentStokesKernel::IncoherentStokesKernel(const Parset &ps, cl::CommandQueue &queue,
            cl::Program &program, cl::Buffer &devIncoherentStokes, cl::Buffer &devInputSamples)
            :
        Kernel(ps, program, "incoherentStokes")
        {
            setArg(0, devIncoherentStokes);
            setArg(1, devInputSamples);

            unsigned nrTimes = ps.nrSamplesPerChannel() / ps.incoherentStokesTimeIntegrationFactor();
            size_t maxNrThreads;
            getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
            unsigned nrPasses = (nrTimes + maxNrThreads - 1) / maxNrThreads;
            unsigned nrTimesPerPass = (nrTimes + nrPasses - 1) / nrPasses;
            globalWorkSize = cl::NDRange(nrTimesPerPass * nrPasses, ps.nrChannelsPerSubband());
            localWorkSize  = cl::NDRange(nrTimesPerPass, 1);

            nrOperations   = ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrStations() * (ps.nrIncoherentStokes() == 1 ? 8 : 20 + 2.0 / ps.incoherentStokesTimeIntegrationFactor());
            nrBytesRead    = (size_t) ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>);
            nrBytesWritten = (size_t) ps.nrIncoherentStokes() * nrTimes * ps.nrChannelsPerSubband() * sizeof(float);
        }

    }
}
