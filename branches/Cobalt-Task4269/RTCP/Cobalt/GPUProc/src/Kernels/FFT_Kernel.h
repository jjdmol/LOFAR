#ifndef GPUPROC_FFT_KERNEL_H
#define GPUPROC_FFT_KERNEL_H

#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "PerformanceCounter.h"
#include "OpenCL_Support.h"
#include "FFT_Plan.h"

namespace LOFAR
{
    namespace RTCP 
    {
         class FFT_Kernel
        {
        public:
            FFT_Kernel(cl::Context &context, unsigned fftSize,
                unsigned nrFFTs, bool forward, cl::Buffer &buffer);
            void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter);


        private:
            unsigned	 nrFFTs, fftSize;
#if defined USE_CUSTOM_FFT
            cl::Kernel	 kernel;
#else
            clFFT_Direction direction;
            FFT_Plan     plan;
            cl::Buffer	 &buffer;
#endif 
            cl::Event	 event;
        };
    }
}
#endif

