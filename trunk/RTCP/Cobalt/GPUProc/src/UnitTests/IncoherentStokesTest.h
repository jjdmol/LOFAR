#ifndef GPUPROC_INCOHERENTSTOKESTEST_H
#define GPUPROC_INCOHERENTSTOKESTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/IncoherentStokesKernel.h"

namespace LOFAR
{
    namespace RTCP 
    {        

        struct IncoherentStokesTest : public UnitTest
        {
            IncoherentStokesTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/IncoherentStokes.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 14 && ps.nrSamplesPerChannel() >= 108) {
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<float, 3> stokesData(boost::extents[ps.nrIncoherentStokes()][ps.nrSamplesPerChannel() / ps.incoherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    IncoherentStokesKernel kernel(ps, queue, program, stokesData, inputData);

                    inputData[4][13][107][0] = std::complex<float>(2, 3);
                    inputData[4][13][107][1] = std::complex<float>(4, 5);

                    inputData.hostToDevice(CL_FALSE);
                    kernel.enqueue(queue, counter);
                    stokesData.deviceToHost(CL_TRUE);

                    const static float expected[] = { 54, -28, 46, 4 };

                    for (unsigned stokes = 0; stokes < ps.nrIncoherentStokes(); stokes ++)
                        check(stokesData[stokes][107 / ps.incoherentStokesTimeIntegrationFactor()][13], expected[stokes]);
                }
            }
        };
    }
}
#endif
