#include "lofar_config.h"    

#include "Kernel.h"
#include "UHEP_BeamFormerKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {      
        UHEP_BeamFormerKernel::UHEP_BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages, cl::Buffer &devInputSamples, cl::Buffer &devBeamFormerWeights)
            :
        Kernel(ps, program, "complexVoltages")
        {
            setArg(0, devComplexVoltages);
            setArg(1, devInputSamples);
            setArg(2, devBeamFormerWeights);

#if 1
            globalWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), ps.nrSubbands());
            localWorkSize  = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), 1);

            size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
            size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(0) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
            size_t nrSampleBytes = count * ps.nrStations() * ps.nrBytesPerComplexSample();
            size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(0) * sizeof(std::complex<float>);
            unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);
            nrOperations   = count * ps.nrStations() * ps.nrTABs(0) * 8;
            nrBytesRead    = nrWeightsBytes + nrSampleBytes + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
            nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
#else
            ASSERT(ps.nrTABs(0) % 3 == 0);
            ASSERT(ps.nrStations() % 6 == 0);
            unsigned nrThreads = NR_POLARIZATIONS * (ps.nrTABs(0) / 3) * (ps.nrStations() / 6);
            globalWorkSize = cl::NDRange(nrThreads, ps.nrSubbands());
            localWorkSize  = cl::NDRange(nrThreads, 1);
            //globalWorkSize = cl::NDRange(ps.nrStations() / 6, ps.nrTABs(0) / 3, ps.nrSubbands());
            //localWorkSize  = cl::NDRange(ps.nrStations() / 6, ps.nrTABs(0) / 3, 1);

            size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
            size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(0) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
            size_t nrSampleBytes = count * ps.nrStations() * ps.nrBytesPerComplexSample();
            size_t nrComplexVoltagesBytes = count * ps.nrTABs(0) * sizeof(std::complex<float>);
            nrOperations   = count * ps.nrStations() * ps.nrTABs(0) * 8;
            nrBytesRead    = nrWeightsBytes + nrSampleBytes;
            nrBytesWritten = nrComplexVoltagesBytes;
#endif
        }
    }
}
