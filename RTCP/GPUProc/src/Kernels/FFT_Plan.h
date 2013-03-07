#ifndef GPUPROC_FFT_PLAN_H
#define GPUPROC_FFT_PLAN_H

#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "PerformanceCounter.h"
#include "OpenCL_Support.h"
#include "OpenCL_FFT/clFFT.h"

namespace LOFAR
{
    namespace RTCP 
    {

        class FFT_Plan
        {
        public:
            FFT_Plan(cl::Context &context, unsigned fftSize);
            ~FFT_Plan();
            clFFT_Plan plan;
        };
    }
}
#endif
