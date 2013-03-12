#ifndef GPUPROC_INTTOFLOATTEST_H
#define GPUPROC_INTTOFLOATTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/IntToFloatKernel.h"

namespace LOFAR
{
  namespace RTCP
  {
    struct IntToFloatTest : public UnitTest
    {
      IntToFloatTest(const Parset &ps)
        :
        UnitTest(ps, "BeamFormer/IntToFloat.cl")
      {
        if (ps.nrStations() >= 3 && ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() >= 10077) {
          MultiArraySharedBuffer<char, 4> inputData(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
          MultiArraySharedBuffer<std::complex<float>, 3> outputData(boost::extents[ps.nrStations()][NR_POLARIZATIONS][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
          IntToFloatKernel kernel(ps, queue, program, outputData, inputData);

          switch (ps.nrBytesPerComplexSample()) {
          case 4: reinterpret_cast<std::complex<short> &>(inputData[2][10076][1][0]) = 7;
            break;

          case 2: reinterpret_cast<std::complex<signed char> &>(inputData[2][10076][1][0]) = 7;
            break;

          case 1: reinterpret_cast<i4complex &>(inputData[2][10076][1][0]) = i4complex(7, 0);
            break;
          }

          inputData.hostToDevice(CL_FALSE);
          kernel.enqueue(queue, counter);
          outputData.deviceToHost(CL_TRUE);
          check(outputData[2][1][10076], std::complex<float>(7.0f, 0));
        }
      }
    };

  }
}
#endif
