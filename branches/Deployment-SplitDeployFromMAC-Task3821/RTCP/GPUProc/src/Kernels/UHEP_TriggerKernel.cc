#include "lofar_config.h"    

#include "Kernel.h"
#include "UHEP_TriggerKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"

namespace LOFAR
{
    namespace RTCP 
    {  
        UHEP_TriggerKernel::UHEP_TriggerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devTriggerInfo, cl::Buffer &devInvFIRfilteredData)
            :
        Kernel(ps, program, "trigger")
        {
            setArg(0, devTriggerInfo);
            setArg(1, devInvFIRfilteredData);

            globalWorkSize = cl::NDRange(16, 16, ps.nrTABs(0));
            localWorkSize  = cl::NDRange(16, 16, 1);

            nrOperations   = (size_t) ps.nrTABs(0) * ps.nrSamplesPerChannel() * 1024 * (3 /* power */ + 2 /* window */ + 1 /* max */ + 7 /* mean/variance */);
            nrBytesRead    = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * 1024 * sizeof(float);
            nrBytesWritten = (size_t) ps.nrTABs(0) * sizeof(TriggerInfo);
        }

    }
}
