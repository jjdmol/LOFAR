#ifndef GPUPROC_BEAMFORMERTRANSPOSETEST_H
#define GPUPROC_BEAMFORMERTRANSPOSETEST_H
#include "CL/cl.hpp"
#include "Interface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/BeamFormerTransposeKernel.h"

namespace LOFAR
{
    namespace RTCP 
    {  
        struct BeamFormerTransposeTest : public UnitTest
        {
            BeamFormerTransposeTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/Transpose.cl")
            {
                if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(0) >= 5) {
                    MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    BeamFormerTransposeKernel transpose(ps, program, transposedData, complexVoltages);

                    complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

                    complexVoltages.hostToDevice(CL_FALSE);
                    transpose.enqueue(queue, counter);
                    transposedData.deviceToHost(CL_TRUE);

                    check(transposedData[4][1][174][18], std::complex<float>(24, 42));
                }
            }
        };
    }
}
#endif
