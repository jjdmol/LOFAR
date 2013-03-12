#ifndef GPUPROC_UHEP_RWORKQUEUE_H
#define GPUPROC_UHEP_RWORKQUEUE_H

#include "lofar_config.h"

#include "CL/cl.hpp"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>
#include <complex>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "BandPass.h"
#include "Pipelines/UHEP_Pipeline.h"
#include "WorkQueue.h"

#include "Kernels/UHEP_TriggerKernel.h"
namespace LOFAR
{
  namespace Cobalt
  {
    class UHEP_WorkQueue : public WorkQueue
    {
    public:
      UHEP_WorkQueue(UHEP_Pipeline &, unsigned queueNumber);

      void doWork(const float *delaysAtBegin, const float *delaysAfterEnd, const float *phaseOffsets);

      UHEP_Pipeline       &pipeline;
      cl::Event inputSamplesEvent, beamFormerWeightsEvent;

      cl::Buffer devBuffers[2];
      cl::Buffer devInputSamples;
      MultiArrayHostBuffer<char, 5> hostInputSamples;

      cl::Buffer devBeamFormerWeights;
      MultiArrayHostBuffer<std::complex<float>, 3> hostBeamFormerWeights;

      cl::Buffer devComplexVoltages;
      cl::Buffer devReverseSubbandMapping;
      cl::Buffer devFFTedData;
      cl::Buffer devInvFIRfilteredData;
      cl::Buffer devInvFIRfilterWeights;

      cl::Buffer devTriggerInfo;
      VectorHostBuffer<TriggerInfo> hostTriggerInfo;
    };
  }
}
#endif
