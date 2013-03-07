#include "lofar_config.h"
#define __CL_ENABLE_EXCEPTIONS
#include "FFT_Kernel.h"

#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "PerformanceCounter.h"
#include "OpenCL_Support.h"
#include "FFT_Plan.h"

namespace LOFAR
{
    namespace RTCP 
    {        

        FFT_Kernel::FFT_Kernel(cl::Context &context, unsigned fftSize, unsigned nrFFTs, bool forward, cl::Buffer &buffer)
            :
        nrFFTs(nrFFTs),
            fftSize(fftSize)
#if defined USE_CUSTOM_FFT
        {
            ASSERT(fftSize == 256);
            ASSERT(forward);
            std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
            cl::Program program = createProgram(context, devices, "FFT.cl", "");
            kernel = cl::Kernel(program, "fft0");
            kernel.setArg(0, buffer);
        }
#else
            , direction(forward ? clFFT_Forward : clFFT_Inverse),
            plan(context, fftSize),
            buffer(buffer)
        {
        }
#endif

        void FFT_Kernel::enqueue(cl::CommandQueue &queue, PerformanceCounter &counter)
        {
#if defined USE_CUSTOM_FFT
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nrFFTs * 64 / 4, 4), cl::NDRange(64, 4), 0, &event);
#else
            cl_int error = clFFT_ExecuteInterleaved(queue(), plan.plan, nrFFTs, direction, buffer(), buffer(), 0, 0, &event());

            if (error != CL_SUCCESS)
                throw cl::Error(error, "clFFT_ExecuteInterleaved");
#endif

            counter.doOperation(event,
                (size_t) nrFFTs * 5 * fftSize * log2(fftSize),
                (size_t) nrFFTs * fftSize * sizeof(std::complex<float>),
                (size_t) nrFFTs * fftSize * sizeof(std::complex<float>));
        }

    }
}
