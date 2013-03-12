#include "lofar_config.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "WorkQueues/BeamFormerWorkQueue.h"
#include "BeamFormerPipeline.h"


namespace LOFAR
{
  namespace RTCP
  {
    BeamFormerPipeline::BeamFormerPipeline(const Parset &ps)
      :
      Pipeline(ps),
      intToFloatCounter("int-to-float", profiling),
      fftCounter("FFT", profiling),
      delayAndBandPassCounter("delay/bp", profiling),
      beamFormerCounter("beamformer", profiling),
      transposeCounter("transpose", profiling),
      dedispersionForwardFFTcounter("ddisp.fw.FFT", profiling),
      dedispersionChirpCounter("chirp", profiling),
      dedispersionBackwardFFTcounter("ddisp.bw.FFT", profiling),
      samplesCounter("samples", profiling)
    {
      double startTime = omp_get_wtime();

#pragma omp parallel sections
      {
#pragma omp section
        intToFloatProgram = createProgram("BeamFormer/IntToFloat.cl");
#pragma omp section
        delayAndBandPassProgram = createProgram("DelayAndBandPass.cl");
#pragma omp section
        beamFormerProgram = createProgram("BeamFormer/BeamFormer.cl");
#pragma omp section
        transposeProgram = createProgram("BeamFormer/Transpose.cl");
#pragma omp section
        dedispersionChirpProgram = createProgram("BeamFormer/Dedispersion.cl");
      }

      LOG_DEBUG_STR("compile time = " << omp_get_wtime() - startTime);
    }

    void BeamFormerPipeline::doWork()
    {
#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
      BeamFormerWorkQueue(*this, omp_get_thread_num() % nrGPUs).doWork();
    }
  }
}

