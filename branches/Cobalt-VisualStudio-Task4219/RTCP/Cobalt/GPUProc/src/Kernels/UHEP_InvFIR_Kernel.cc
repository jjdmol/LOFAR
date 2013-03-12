#include "lofar_config.h"    

#include "Kernel.h"
#include "UHEP_InvFIR_Kernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {  
        UHEP_InvFIR_Kernel::UHEP_InvFIR_Kernel(const Parset &ps, cl::CommandQueue &queue,
            cl::Program &program, cl::Buffer &devInvFIRfilteredData, cl::Buffer &devFFTedData, 
            cl::Buffer &devInvFIRfilterWeights)
            :
        Kernel(ps, program, "invFIRfilter")
        {
            setArg(0, devInvFIRfilteredData);
            setArg(1, devFFTedData);
            setArg(2, devInvFIRfilterWeights);

            size_t maxNrThreads, nrThreads;
            getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
            // round down to nearest power of two
            for (nrThreads = 1024; nrThreads > maxNrThreads; nrThreads /= 2)
                ;

            globalWorkSize = cl::NDRange(1024, NR_POLARIZATIONS, ps.nrTABs(0));
            localWorkSize  = cl::NDRange(nrThreads, 1, 1);

            size_t count = ps.nrTABs(0) * NR_POLARIZATIONS * 1024;
            nrOperations   = count * ps.nrSamplesPerChannel() * NR_STATION_FILTER_TAPS * 2;
            nrBytesRead    = count * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * sizeof(float);
            nrBytesWritten = count * ps.nrSamplesPerChannel() * sizeof(float);
        }
    }
}
