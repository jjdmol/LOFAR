#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "FFT_Plan.h"

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "PerformanceCounter.h"
#include "OpenCL_Support.h"
#include "OpenCL_FFT/clFFT.h"

namespace LOFAR
{
  namespace RTCP
  {
    FFT_Plan::FFT_Plan(cl::Context &context, unsigned fftSize)
    {
      clFFT_Dim3 dim = { fftSize, 1, 1 };
      cl_int error;
      plan = clFFT_CreatePlan(context(), dim, clFFT_1D, clFFT_InterleavedComplexFormat, &error);

      if (error != CL_SUCCESS)
        throw cl::Error(error, "clFFT_CreatePlan");

      //clFFT_DumpPlan(plan, stdout);
    }

    FFT_Plan::~FFT_Plan()
    {
      clFFT_DestroyPlan(plan);
    }


  }
}
