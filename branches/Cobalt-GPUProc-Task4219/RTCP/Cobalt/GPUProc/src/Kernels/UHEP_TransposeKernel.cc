#include "lofar_config.h"    

#include "Kernel.h"
#include "UHEP_TransposeKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {      
        UHEP_TransposeKernel::UHEP_TransposeKernel(const Parset &ps, cl::Program &program, cl::Buffer &devFFTedData, cl::Buffer &devComplexVoltages, cl::Buffer &devReverseSubbandMapping)
            :
        Kernel(ps, program, "UHEP_Transpose")
        {
            setArg(0, devFFTedData);
            setArg(1, devComplexVoltages);
            setArg(2, devReverseSubbandMapping);

            globalWorkSize = cl::NDRange(256, (ps.nrTABs(0) + 15) / 16, 512 / 16);
            localWorkSize  = cl::NDRange(256, 1, 1);

            nrOperations   = 0;
            nrBytesRead    = (size_t) ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>);
            nrBytesWritten = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * 512 * sizeof(std::complex<float>);
        }


    }
}
