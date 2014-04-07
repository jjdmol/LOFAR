#ifndef GPUPROC_FFTTEST_H
#define GPUPROC_FFTTEST_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernel.h"
#include <iostream>

namespace LOFAR
{
    namespace RTCP 
    {   
        
#if 0
        struct FFT_Test : public UnitTest
        {
            FFT_Test(const Parset &ps)
                : UnitTest(ps, "fft.cl")
            {
                MultiArraySharedBuffer<std::complex<float>, 1> in(boost::extents[8], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                MultiArraySharedBuffer<std::complex<float>, 1> out(boost::extents[8], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);

                for (unsigned i = 0; i < 8; i ++)
                    in[i] = std::complex<float>(2 * i + 1, 2 * i + 2);

                clAmdFftSetupData setupData;
                cl::detail::errHandler(clAmdFftInitSetupData(&setupData), "clAmdFftInitSetupData");
                setupData.debugFlags = CLFFT_DUMP_PROGRAMS;
                cl::detail::errHandler(clAmdFftSetup(&setupData), "clAmdFftSetup");

                clAmdFftPlanHandle plan;
                size_t dim[1] = { 8 };

                cl::detail::errHandler(clAmdFftCreateDefaultPlan(&plan, context(), CLFFT_1D, dim), "clAmdFftCreateDefaultPlan");
                cl::detail::errHandler(clAmdFftSetResultLocation(plan, CLFFT_OUTOFPLACE), "clAmdFftSetResultLocation");
                cl::detail::errHandler(clAmdFftSetPlanBatchSize(plan, 1), "clAmdFftSetPlanBatchSize");
                cl::detail::errHandler(clAmdFftBakePlan(plan, 1, &queue(), 0, 0), "clAmdFftBakePlan");

                in.hostToDevice(CL_FALSE);
                cl_mem ins[1] = { ((cl::Buffer) in)() };
                cl_mem outs[1] = { ((cl::Buffer) out)() };
#if 1
                cl::detail::errHandler(clAmdFftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue(), 0, 0, 0, ins, outs, 0), "clAmdFftEnqueueTransform");
#else
                cl::Kernel kernel(program, "fft_fwd");
                kernel.setArg(0, (cl::Buffer) in);
                kernel.setArg(1, (cl::Buffer) out);
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(64, 1, 1), cl::NDRange(64, 1, 1));
#endif
                out.deviceToHost(CL_TRUE);

                for (unsigned i = 0; i < 8; i ++)
                    std::cout << out[i] << std::endl;

                cl::detail::errHandler(clAmdFftDestroyPlan(&plan), "clAmdFftDestroyPlan");
                cl::detail::errHandler(clAmdFftTeardown(), "clAmdFftTeardown");
            }
        };
#endif
    }
}
#endif
