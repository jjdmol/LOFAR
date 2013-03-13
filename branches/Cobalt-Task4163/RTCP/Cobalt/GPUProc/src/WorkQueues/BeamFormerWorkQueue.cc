#include "lofar_config.h"

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "CoInterface/Parset.h"
#include "OpenCL_Support.h"
#include "OpenMP_Support.h"
#include <algorithm>
#include <iostream>
#include "ApplCommon/PosixTime.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Pipeline.h"
#include "BeamFormerWorkQueue.h"
#include "BandPass.h"

#include "Kernels/IntToFloatKernel.h"
#include "Kernels/Filter_FFT_Kernel.h"
#include "Kernels/DelayAndBandPassKernel.h"
#include "Kernels/BeamFormerKernel.h"
#include "Kernels/BeamFormerTransposeKernel.h"
#include "Kernels/DedispersionForwardFFTkernel.h"
#include "Kernels/DedispersionBackwardFFTkernel.h"
#include "Kernels/DedispersionChirpKernel.h"

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerWorkQueue::BeamFormerWorkQueue(BeamFormerPipeline &pipeline, unsigned gpuNumber)
      :
      WorkQueue( pipeline.context,pipeline.devices[gpuNumber], gpuNumber, pipeline.ps),
      pipeline(pipeline),
      inputSamples(boost::extents[ps.nrStations()][ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband()][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devFilteredData(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
      bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      delaysAtBegin(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      delaysAfterEnd(boost::extents[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      phaseOffsets(boost::extents[ps.nrBeams()][NR_POLARIZATIONS], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devCorrectedData(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>))),
      beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devComplexVoltages(cl::Buffer(pipeline.context, CL_MEM_READ_WRITE, ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>))),
      //transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE)
      transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE),
      DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY),

      intToFloatKernel(ps, queue, pipeline.intToFloatProgram, devFilteredData, inputSamples),
      fftKernel(ps, pipeline.context, devFilteredData),
      delayAndBandPassKernel(ps, pipeline.delayAndBandPassProgram, devCorrectedData, devFilteredData, delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassCorrectionWeights),
      beamFormerKernel(ps, pipeline.beamFormerProgram, devComplexVoltages, devCorrectedData, beamFormerWeights),
      transposeKernel(ps, pipeline.transposeProgram, transposedComplexVoltages, devComplexVoltages),
      dedispersionForwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionBackwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionChirpKernel(ps, pipeline.dedispersionChirpProgram, queue, transposedComplexVoltages, DMs)

    {
      if (ps.correctBandPass()) {
        BandPass::computeCorrectionFactors(bandPassCorrectionWeights.origin(), ps.nrChannelsPerSubband());
        bandPassCorrectionWeights.hostToDevice(CL_TRUE);
      }
    }


    void BeamFormerWorkQueue::doWork()
    {
      //queue.enqueueWriteBuffer(devFIRweights, CL_TRUE, 0, firWeightsSize, firFilterWeights);
      bandPassCorrectionWeights.hostToDevice(CL_TRUE);
      DMs.hostToDevice(CL_TRUE);

      double startTime = ps.startTime(), currentTime, stopTime = ps.stopTime(), blockTime = ps.CNintegrationTime();

#pragma omp barrier

      double executionStartTime = omp_get_wtime();

      for (unsigned block = 0; (currentTime = startTime + block * blockTime) < stopTime; block++) {
#pragma omp single nowait
        LOG_INFO_STR("block = " << block << ", time = " << to_simple_string(from_ustime_t(currentTime)));

        memset(delaysAtBegin.origin(), 0, delaysAtBegin.bytesize());
        memset(delaysAfterEnd.origin(), 0, delaysAfterEnd.bytesize());
        memset(phaseOffsets.origin(), 0, phaseOffsets.bytesize());

        // FIXME!!!
        if (ps.nrStations() >= 3)
          delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

        delaysAtBegin.hostToDevice(CL_FALSE);
        delaysAfterEnd.hostToDevice(CL_FALSE);
        phaseOffsets.hostToDevice(CL_FALSE);
        beamFormerWeights.hostToDevice(CL_FALSE);

#pragma omp for schedule(dynamic), nowait
        for (unsigned subband = 0; subband < ps.nrSubbands(); subband++) {
#if 1
          {
#if defined USE_B7015
            OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
            inputSamples.hostToDevice(CL_TRUE);
            pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
          }
#endif

          //#pragma omp critical (GPU)
          {
            if (ps.nrChannelsPerSubband() > 1) {
              intToFloatKernel.enqueue(queue, pipeline.intToFloatCounter);
              fftKernel.enqueue(queue, pipeline.fftCounter);
            }

            delayAndBandPassKernel.enqueue(queue, pipeline.delayAndBandPassCounter, subband);
            beamFormerKernel.enqueue(queue, pipeline.beamFormerCounter);
            transposeKernel.enqueue(queue, pipeline.transposeCounter);
            dedispersionForwardFFTkernel.enqueue(queue, pipeline.dedispersionForwardFFTcounter);
            dedispersionChirpKernel.enqueue(queue, pipeline.dedispersionChirpCounter, ps.subbandToFrequencyMapping()[subband]);
            dedispersionBackwardFFTkernel.enqueue(queue, pipeline.dedispersionBackwardFFTcounter);

            queue.finish();
          }

          //queue.enqueueReadBuffer(devComplexVoltages, CL_TRUE, 0, hostComplexVoltages.bytesize(), hostComplexVoltages.origin());
          //dedispersedData.deviceToHost(CL_TRUE);
        }
      }

#pragma omp barrier

#pragma omp master
      if (!profiling)
        LOG_INFO_STR("run time = " << omp_get_wtime() - executionStartTime);
    }
  }
}
