#include "lofar_config.h"    

#include "Kernel.h"
#include "IntToFloatKernel.h"

#include "Interface/Parset.h"
#include "OpenCL_Support.h"

namespace LOFAR
{
    namespace RTCP 
    {
        IntToFloatKernel::IntToFloatKernel(const Parset &ps, cl::CommandQueue &queue, cl::Program &program, cl::Buffer &devFilteredData, cl::Buffer &devInputSamples)
            :
        Kernel(ps, program, "intToFloat")
        {
            setArg(0, devFilteredData);
            setArg(1, devInputSamples);

            size_t maxNrThreads;
            getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
            globalWorkSize = cl::NDRange(maxNrThreads, ps.nrStations());
            localWorkSize  = cl::NDRange(maxNrThreads, 1);

            size_t nrSamples = ps.nrStations() * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
            nrOperations   = nrSamples * 2;
            nrBytesRead    = nrSamples * 2 * ps.nrBitsPerSample() / 8;
            nrBytesWritten = nrSamples * sizeof(std::complex<float>);
        }


    }
}

