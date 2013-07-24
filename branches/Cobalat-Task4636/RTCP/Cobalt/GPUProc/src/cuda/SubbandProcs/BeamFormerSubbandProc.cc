//# BeamFormerSubbandProc.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "BeamFormerSubbandProc.h"

#include <Common/LofarLogger.h>
#include <ApplCommon/PosixTime.h>
#include <CoInterface/Parset.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerSubbandProc::BeamFormerSubbandProc(const Parset &parset,
      gpu::Context &context)
    :
      SubbandProc( parset, context ),
      prevBlock(-1),
      prevSAP(-1),

      // NOTE: Make sure the history samples are dealt with properly until the
      // FIR, which the beam former does in a later stage!
      //
      // NOTE: Sizes are probably completely wrong.
      devInput(1,//IntToFloatKernel::bufferSize(ps, IntToFloatKernel::INPUT_DATA),
               1,//DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::DELAYS),
               1,//DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::PHASE_OFFSETS),
               context),
      devFilteredData(context, 1)//DelayAndBandPassKernel::bufferSize(ps, DelayAndBandPassKernel::INPUT_DATA))
#if 0
      firFilterKernel(ps, programs.firFilterProgram,
                      devFilteredData, devInput.inputSamples, devFIRweights),
      fftKernel(ps, context, devFilteredData),
      delayAndBandPassKernel(ps, programs.delayAndBandPassProgram,
                             devInput.inputSamples,
                             devFilteredData,
                             devInput.delaysAtBegin,
                             devInput.delaysAfterEnd,
                             devInput.phaseOffsets,
                             devBandPassCorrectionWeights)
#endif
#if 0
      devFilteredData(queue, CL_MEM_READ_WRITE, ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * sizeof(std::complex<float>)),
      bandPassCorrectionWeights(boost::extents[ps.nrChannelsPerSubband()], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devCorrectedData(queue, CL_MEM_READ_WRITE, ps.nrStations() * ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * NR_POLARIZATIONS * sizeof(std::complex<float>)),
      beamFormerWeights(boost::extents[ps.nrStations()][ps.nrChannelsPerSubband()][ps.nrTABs(0)], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY),
      devComplexVoltages(queue, CL_MEM_READ_WRITE, ps.nrChannelsPerSubband() * ps.nrSamplesPerChannel() * ps.nrTABs(0) * NR_POLARIZATIONS * sizeof(std::complex<float>)),
      //transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrSamplesPerChannel()][ps.nrChannelsPerSubband()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE)
      transposedComplexVoltages(boost::extents[ps.nrTABs(0)][NR_POLARIZATIONS][ps.nrChannelsPerSubband()][ps.nrSamplesPerChannel()], queue, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE),
      DMs(boost::extents[ps.nrTABs(0)], queue, CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY),

      intToFloatKernel(ps, queue, pipeline.intToFloatProgram, devFilteredData, inputSamples),
      beamFormerKernel(ps, pipeline.beamFormerProgram, devComplexVoltages, devCorrectedData, beamFormerWeights),
      transposeKernel(ps, pipeline.transposeProgram, transposedComplexVoltages, devComplexVoltages),
      dedispersionForwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionBackwardFFTkernel(ps, pipeline.context, transposedComplexVoltages),
      dedispersionChirpKernel(ps, pipeline.dedispersionChirpProgram, queue, transposedComplexVoltages, DMs)
#endif
    {
      // put enough objects in the outputPool to operate
      for (size_t i = 0; i < 3; ++i) {
        outputPool.free.append(new BeamFormedData(
                4,
                ps.nrChannelsPerSubband(),
                ps.integrationSteps(),
                context));
      }

      // CPU timers are set by CorrelatorPipeline
      addTimer("CPU - read input");
      addTimer("CPU - process");
      addTimer("CPU - postprocess");
      addTimer("CPU - total");

      // GPU timers are set by us
      addTimer("GPU - total");
      addTimer("GPU - input");
      addTimer("GPU - output");
      addTimer("GPU - compute");
      addTimer("GPU - wait");
    }


    void BeamFormerSubbandProc::processSubband(SubbandProcInputData &input, StreamableData &_output)
    {
      (void)_output;

      size_t block = input.blockID.block;
      unsigned subband = input.blockID.globalSubbandIdx;

      {
#if defined USE_B7015
        OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
        queue.writeBuffer(devInput.inputSamples, input.inputSamples, true);
//      counters["input - samples"]->doOperation(input.inputSamples.deviceBuffer.event, 0, 0, input.inputSamples.bytesize());
      }

      if (ps.delayCompensation())
      {
        unsigned SAP = ps.settings.subbands[subband].SAP;

        // Only upload delays if they changed w.r.t. the previous subband.
        if ((int)SAP != prevSAP || (ssize_t)block != prevBlock) {
          queue.writeBuffer(devInput.delaysAtBegin,  input.delaysAtBegin,  false);
          queue.writeBuffer(devInput.delaysAfterEnd, input.delaysAfterEnd, false);
          queue.writeBuffer(devInput.phaseOffsets,   input.phaseOffsets,   false);
          // beamFormerWeights.hostToDevice(false);

          prevSAP = SAP;
          prevBlock = block;
        }
      }

      // intToFloatKernel.enqueue(queue);
      // fftKernel.enqueue();
      // delayAndBandpassKernel.enqueue()
      // beamFormerKernel.enqueue()
      // transposeKernel.enqueue()
      // inverseFFTKernel.enqueue()
      // PPFKernel.enqueue()

#if 0
      DMs.hostToDevice(true);


          {
#if defined USE_B7015
            OMP_ScopedLock scopedLock(pipeline.hostToDeviceLock[gpu / 2]);
#endif
            inputSamples.hostToDevice(true);
//            pipeline.samplesCounter.doOperation(inputSamples.event, 0, 0, inputSamples.bytesize());
          }

          {
            if (ps.nrChannelsPerSubband() > 1) {
              intToFloatKernel.enqueue(queue/*, pipeline.intToFloatCounter*/);
              fftKernel.enqueue(queue/*, pipeline.fftCounter*/);
            }

            delayAndBandPassKernel.enqueue(queue/*, pipeline.delayAndBandPassCounter*/, subband);
            beamFormerKernel.enqueue(queue/*, pipeline.beamFormerCounter*/);
            transposeKernel.enqueue(queue/*, pipeline.transposeCounter*/);
            dedispersionForwardFFTkernel.enqueue(queue/*, pipeline.dedispersionForwardFFTcounter*/);
            dedispersionChirpKernel.enqueue(queue/*, pipeline.dedispersionChirpCounter*/, ps.subbandToFrequencyMapping()[subband]);
            dedispersionBackwardFFTkernel.enqueue(queue/*, pipeline.dedispersionBackwardFFTcounter*/);

            queue.synchronize();
          }

          //queue.enqueueReadBuffer(devComplexVoltages, CL_TRUE, 0, hostComplexVoltages.bytesize(), hostComplexVoltages.origin());
          //dedispersedData.deviceToHost(true);
        }
      }
#endif
    }

    void BeamFormerSubbandProc::postprocessSubband(StreamableData &_output)
    {
      (void)_output;
    }
  }
}

