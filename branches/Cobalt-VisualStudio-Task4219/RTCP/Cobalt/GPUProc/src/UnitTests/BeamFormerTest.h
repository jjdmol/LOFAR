#ifndef GPUPROC_BEAMFORMERTEST_H
#define GPUPROC_BEAMFORMERTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/BeamFormerKernel.h"

namespace LOFAR
{
    namespace RTCP 
    {  
        struct BeamFormerTest : public UnitTest
        {
            BeamFormerTest(const Parset &ps)
                :
            UnitTest(ps, "BeamFormer/BeamFormer.cl")
            {
                if (ps.nrStations() >= 5 && ps.nrSamplesPerChannel() >= 13 && ps.nrChannelsPerSubband() >= 7 && ps.nrTABs(0) >= 6) {
                    MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 3> beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE);
                    BeamFormerKernel beamFormer(ps, program, complexVoltages, inputData, beamFormerWeights);

                    inputData[4][6][12][1] = std::complex<float>(2.2, 3);
                    beamFormerWeights[4][6][5] = std::complex<float>(4, 5);

                    inputData.hostToDevice(CL_FALSE);
                    beamFormerWeights.hostToDevice(CL_FALSE);
                    beamFormer.enqueue(queue, counter);
                    complexVoltages.deviceToHost(CL_TRUE);

                    check(complexVoltages[6][12][5][1], std::complex<float>(-6.2, 23));

#if 0
                    for (unsigned tab = 0; tab < ps.nrTABs(0); tab ++)
                        for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
                            for (unsigned ch = 0; ch < ps.nrChannelsPerSubband(); ch ++)
                                for (unsigned t = 0; t < ps.nrSamplesPerChannel(); t ++)
                                    if (complexVoltages[tab][pol][ch][t] != std::complex<float>(0, 0))
                                        std::cout << "complexVoltages[" << tab << "][" << pol << "][" << ch << "][" << t << "] = " << complexVoltages[tab][pol][ch][t] << std::endl;
#endif
                }
            }
        };
    }
}
#endif