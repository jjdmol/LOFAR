
#include "lofar_config.h"    

#include "Kernel.h"
#include "BeamFormerKernel.h"

#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
    namespace RTCP 
    {      
            BeamFormerKernel::BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages, cl::Buffer &devCorrectedData, cl::Buffer &devBeamFormerWeights)
                :
            Kernel(ps, program, "complexVoltages")
            {
                setArg(0, devComplexVoltages);
                setArg(1, devCorrectedData);
                setArg(2, devBeamFormerWeights);

                globalWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), ps.nrChannelsPerSubband());
                localWorkSize  = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), 1);

                // FIXME: nrTABs
                //queue.enqueueNDRangeKernel(*this, cl::NullRange, cl::NDRange(16, ps.nrTABs(0), ps.nrChannelsPerSubband()), cl::NDRange(16, ps.nrTABs(0), 1), 0, &event);

                size_t count = ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS;
                size_t nrWeightsBytes = ps.nrStations() * ps.nrTABs(0) * ps.nrChannelsPerSubband() * NR_POLARIZATIONS * sizeof(std::complex<float>);
                size_t nrSampleBytesPerPass = count * ps.nrStations() * sizeof(std::complex<float>);
                size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(0) * sizeof(std::complex<float>);
                unsigned nrPasses = std::max((ps.nrStations() + 6) / 16, 1U);
                nrOperations   = count * ps.nrStations() * ps.nrTABs(0) * 8;
                nrBytesRead    = nrWeightsBytes + nrSampleBytesPerPass + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
                nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
            }
        
    }
}