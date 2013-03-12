#ifndef GPUPROC_DEDISPERSIONCHIRPTEST_H
#define GPUPROC_DEDISPERSIONCHIRPTEST_H
#include "CL/cl.hpp"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "UnitTest.h"
#include "global_defines.h"
#include <complex>
#include "Kernels/DedispersionChirpKernel.h"

namespace LOFAR
{
  namespace RTCP
  {

    struct DedispersionChirpTest : public UnitTest
    {
      DedispersionChirpTest(const Parset &ps)
        :
        UnitTest(ps, "BeamFormer/Dedispersion.cl")
      {
        if (ps.nrTABs(0) > 3 && ps.nrChannelsPerSubband() > 13 && ps.nrSamplesPerChannel() / ps.dedispersionFFTsize() > 1 && ps.dedispersionFFTsize() > 77) {
          MultiArraySharedBuffer<std::complex<float>, 5> data(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel() / ps.dedispersionFFTsize()][ps.dedispersionFFTsize()], queue, CL_MEM_READ_WRITE, CL_MEM_READ_WRITE);
          MultiArraySharedBuffer<float, 1> DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY);
          DedispersionChirpKernel dedispersionChirpKernel(ps, program, queue, data, DMs);

          data[3][1][13][1][77] = std::complex<float>(2, 3);
          DMs[3] = 2;

          DMs.hostToDevice(CL_FALSE);
          data.hostToDevice(CL_FALSE);
          dedispersionChirpKernel.enqueue(queue, counter, 60e6);
          data.deviceToHost(CL_TRUE);

          std::cout << data[3][1][13][1][77] << std::endl;
        }
      }
    };
  }
}
#endif
