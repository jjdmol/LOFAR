
#include "lofar_config.h"    

#include "Kernel.h"
#include "BeamFormerTransposeKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {      

        BeamFormerTransposeKernel::BeamFormerTransposeKernel(const Parset &ps, cl::Program &program, cl::Buffer &devTransposedData, cl::Buffer &devComplexVoltages)
            :
        Kernel(ps, program, "transposeComplexVoltages")
        {
            ASSERT(ps.nrSamplesPerChannel() % 16 == 0);
            setArg(0, devTransposedData);
            setArg(1, devComplexVoltages);

            //globalWorkSize = cl::NDRange(256, (ps.nrTABs(0) + 15) / 16, (ps.nrChannelsPerSubband() + 15) / 16);
            globalWorkSize = cl::NDRange(256, (ps.nrTABs(0) + 15) / 16, ps.nrSamplesPerChannel() / 16);
            localWorkSize  = cl::NDRange(256, 1, 1);

            nrOperations   = 0;
            nrBytesRead    = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>),
                //nrBytesWritten = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>);
                nrBytesWritten = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * sizeof(std::complex<float>);
        }

    }
}