#include "lofar_config.h"

#include "Kernel.h"
#include "CoherentStokesKernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"

namespace LOFAR
{
  namespace RTCP
  {

    CoherentStokesKernel::CoherentStokesKernel(const Parset &ps, cl::Program &program, cl::Buffer &devStokesData, cl::Buffer &devComplexVoltages)
      :
      Kernel(ps, program, "coherentStokes")
    {
      ASSERT(ps.nrChannelsPerSubband() >= 16 && ps.nrChannelsPerSubband() % 16 == 0);
      ASSERT(ps.nrCoherentStokes() == 1 || ps.nrCoherentStokes() == 4);
      setArg(0, devStokesData);
      setArg(1, devComplexVoltages);

      globalWorkSize = cl::NDRange(256, (ps.nrTABs(0) + 15) / 16, (ps.nrChannelsPerSubband() + 15) / 16);
      localWorkSize = cl::NDRange(256, 1, 1);

      nrOperations = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * (ps.nrCoherentStokes() == 1 ? 8 : 20 + 2.0 / ps.coherentStokesTimeIntegrationFactor());
      nrBytesRead = (size_t) ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>);
      nrBytesWritten = (size_t) ps.nrTABs(0) * ps.nrCoherentStokes() * ps.nrSamplesPerChannel() / ps.coherentStokesTimeIntegrationFactor() * ps.nrChannelsPerSubband() * sizeof(float);
    }

  }
}

