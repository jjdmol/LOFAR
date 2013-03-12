#ifndef GPUPROC_COHERENTSTOKESTEST_H
#define GPUPROC_COHERENTSTOKESTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/CoherentStokesKernel.h"

namespace LOFAR
{
  namespace RTCP
  {
    struct CoherentStokesTest : public UnitTest
    {
      CoherentStokesTest(const Parset &ps)
        :
        UnitTest(ps, "BeamFormer/CoherentStokes.cl")
      {
        if (ps.nrChannelsPerSubband() >= 19 && ps.nrSamplesPerChannel() >= 175 && ps.nrTABs(0) >= 5) {
          MultiArraySharedBuffer<float, 4> stokesData(boost::extents[ps.nrTABs(0)][ps.nrCoherentStokes()][ps.nrSamplesPerChannel() / ps.coherentStokesTimeIntegrationFactor()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
#if 1
          MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()][ps.nrTABs(0)][NR_POLARIZATIONS], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
          CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

          complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
          complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#else
          MultiArraySharedBuffer<std::complex<float>, 4> complexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_ONLY);
          CoherentStokesKernel stokesKernel(ps, program, stokesData, complexVoltages);

          complexVoltages[18][174][4][0] = std::complex<float>(2, 3);
          complexVoltages[18][174][4][1] = std::complex<float>(4, 5);
#endif

          complexVoltages.hostToDevice(CL_FALSE);
          stokesKernel.enqueue(queue, counter);
          stokesData.deviceToHost(CL_TRUE);

          for (unsigned stokes = 0; stokes < ps.nrCoherentStokes(); stokes++)
            std::cout << stokesData[4][stokes][174 / ps.coherentStokesTimeIntegrationFactor()][18] << std::endl;
        }
      }
    };

  }
}
#endif
