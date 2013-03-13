#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "Filter_FFT_Kernel.h"
#include "FFT_Kernel.h"
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"


namespace LOFAR
{
  namespace Cobalt
  {
    Filter_FFT_Kernel::Filter_FFT_Kernel(const Parset &ps, cl::Context &context, cl::Buffer &devFilteredData)
      :
      FFT_Kernel(context, ps.nrChannelsPerSubband(), ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel(), true, devFilteredData)
    {
    }

  }
}
