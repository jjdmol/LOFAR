#ifndef GPUPROC_UHEP_TRANSPOSETEST_H
#define GPUPROC_UHEP_TRANSPOSETEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/UHEP_TransposeKernel.h"
#include "UHEP/InvertedStationPPFWeights.h" // reverseSubbandMapping

namespace LOFAR
{
    namespace RTCP 
    {   
struct UHEP_TransposeTest : public UnitTest
        {
            UHEP_TransposeTest(const Parset &ps)
                :
            UnitTest(ps, "UHEP/Transpose.cl")
            {
                if (ps.nrSubbands() >= 19 && ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1 >= 175 && ps.nrTABs(0) >= 5) {
                    MultiArraySharedBuffer<std::complex<float>, 4> transposedData(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][512], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
                    MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrSubbands()][ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
                    cl::Buffer devReverseSubbandMapping(context, CL_MEM_READ_ONLY, 512 * sizeof(int));
                    UHEP_TransposeKernel transpose(ps, program, transposedData, complexVoltages, devReverseSubbandMapping);

                    complexVoltages[18][174][4][1] = std::complex<float>(24, 42);

                    queue.enqueueWriteBuffer(devReverseSubbandMapping, CL_FALSE, 0, 512 * sizeof(int), reverseSubbandMapping);
                    complexVoltages.hostToDevice(CL_FALSE);
                    transpose.enqueue(queue, counter);
                    transposedData.deviceToHost(CL_TRUE);

                    check(transposedData[4][1][174][38], std::complex<float>(24, 42));
                }
            }
        };
    }
}
#endif
