#ifndef GPUPROC_CORRELATETRIANGLETEST_H
#define GPUPROC_CORRELATETRIANGLETEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/CorrelatorKernel.h"

namespace LOFAR
{
  namespace RTCP
  {
    struct CorrelateTriangleTest : public UnitTest
    {
      CorrelateTriangleTest(const Parset &ps)
        :
        //UnitTest(ps, "Correlator.cl")
        UnitTest(ps, "NewCorrelator.cl")
      {
        if (ps.nrStations() >= 5 && ps.nrChannelsPerSubband() >= 6 && ps.nrSamplesPerChannel() >= 100) {
          MultiArraySharedBuffer<std::complex<float>, 4> visibilities(boost::extents[ps.nrBaselines()][ps.nrChannelsPerSubband()][NR_POLARIZATIONS][NR_POLARIZATIONS], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
          MultiArraySharedBuffer<std::complex<float>, 4> inputData(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);
          CorrelateTriangleKernel correlator(ps, queue, program, visibilities, inputData);

          //inputData[3][5][99][1] = std::complex<float>(3, 4);
          //inputData[4][5][99][1] = std::complex<float>(5, 6);
          inputData[0][5][99][1] = std::complex<float>(3, 4);
          inputData[2][5][99][1] = std::complex<float>(5, 6);

          visibilities.hostToDevice(CL_FALSE);
          inputData.hostToDevice(CL_FALSE);
          correlator.enqueue(queue, counter);
          visibilities.deviceToHost(CL_TRUE);

          //check(visibilities[13][5][1][1], std::complex<float>(39, 2));
          for (unsigned bl = 0; bl < ps.nrBaselines(); bl++)
            if (visibilities[bl][5][1][1] != std::complex<float>(0, 0))
              std::cout << "bl = " << bl << ", visibility = " << visibilities[bl][5][1][1] << std::endl;
        }
      }
    };
  }
}
#endif

