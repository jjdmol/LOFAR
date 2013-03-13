#include "lofar_config.h"

#include "Kernel.h"
#include "UHEP_InvFFT_Kernel.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include <complex>

namespace LOFAR
{
  namespace Cobalt
  {
    UHEP_InvFFT_Kernel::UHEP_InvFFT_Kernel(const Parset &ps, cl::Program &program, cl::Buffer &devFFTedData)
      :
      Kernel(ps, program, "inv_fft")
    {
      setArg(0, devFFTedData);
      setArg(1, devFFTedData);

      globalWorkSize = cl::NDRange(128, ps.nrTABs(0) * NR_POLARIZATIONS * ps.nrSamplesPerChannel());
      localWorkSize = cl::NDRange(128, 1);

      size_t nrFFTs = (size_t) ps.nrTABs(0) * NR_POLARIZATIONS * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1);
      nrOperations = nrFFTs * 5 * 1024 * 10;
      nrBytesRead = nrFFTs * 512 * sizeof(std::complex<float>);
      nrBytesWritten = nrFFTs * 1024 * sizeof(float);
    }

  }
}