//# BeamFormerPreprocessingStep.cc
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

#include "BeamFormerPreprocessingStep.h"
#include "BeamFormerFactories.h"

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

#include <CoInterface/Parset.h>
#include <ApplCommon/PosixTime.h>
#include <Common/LofarLogger.h>

#include <iomanip>

// Set to true to get detailed buffer informatio
#if 0
#define DUMPBUFFER(a,b) dumpBuffer((a),  (b))
#else
#define DUMPBUFFER(a,b)
#endif

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerPreprocessingStep::BeamFormerPreprocessingStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      BeamFormerFactories &factories,
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB,
      boost::shared_ptr<gpu::DeviceMemory> i_devNull)
      :
      BeamFormerSubbandProcStep(parset, i_queue)
    {
      devInput=i_devInput;
      devA=i_devA;
      devB=i_devB;
      devNull=i_devNull;
      initMembers(context, factories);
    }

    BeamFormerPreprocessingStep::~BeamFormerPreprocessingStep()
    {}

    void BeamFormerPreprocessingStep::initMembers(gpu::Context &context,
      BeamFormerFactories &factories){

      // intToFloat: input -> B
      intToFloatBuffers = std::auto_ptr<IntToFloatKernel::Buffers>(
        new IntToFloatKernel::Buffers(*devInput->inputSamples, *devB));

      intToFloatKernel = std::auto_ptr<IntToFloatKernel>(
        factories.intToFloat.create(queue, *intToFloatBuffers));

      // FFTShift: B -> B
      firstFFTShiftBuffers = std::auto_ptr<FFTShiftKernel::Buffers>(
        new FFTShiftKernel::Buffers(*devB, *devB));

      firstFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
        factories.fftShift.create(queue, *firstFFTShiftBuffers));

      // FFT: B -> B
      firstFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(queue,
        ps.settings.beamFormer.nrDelayCompensationChannels,
        (ps.nrStations() * NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
        ps.settings.beamFormer.nrDelayCompensationChannels),
        true, *devB));

      // delayComp: B -> A
      delayCompensationBuffers = std::auto_ptr<DelayAndBandPassKernel::Buffers>(
        new DelayAndBandPassKernel::Buffers(*devB, *devA, devInput->delaysAtBegin,
        devInput->delaysAfterEnd,
        devInput->phase0s, *devNull));

      delayCompensationKernel = std::auto_ptr<DelayAndBandPassKernel>(
        factories.delayCompensation.create(queue, *delayCompensationBuffers));


      // Only perform second FFTshift and FFT if we have to.
      if (ps.settings.beamFormer.nrHighResolutionChannels /
          ps.settings.beamFormer.nrDelayCompensationChannels > 1) {

        // FFTShift: A -> A
        secondFFTShiftBuffers = std::auto_ptr<FFTShiftKernel::Buffers>(
          new FFTShiftKernel::Buffers(*devA, *devA));

        secondFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
          factories.fftShift.create(queue, *secondFFTShiftBuffers));

        // FFT: A -> A
        unsigned secondFFTnrFFTs = ps.nrStations() * NR_POLARIZATIONS *
          ps.nrSamplesPerSubband() /
           (ps.settings.beamFormer.nrHighResolutionChannels /
            ps.settings.beamFormer.nrDelayCompensationChannels);

        secondFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(queue,
          ps.settings.beamFormer.nrHighResolutionChannels /
          ps.settings.beamFormer.nrDelayCompensationChannels,
          secondFFTnrFFTs, true, *devA));
      }

      // bandPass: A -> B
      devBandPassCorrectionWeights = std::auto_ptr<gpu::DeviceMemory>(
        new gpu::DeviceMemory(context,
        factories.bandPassCorrection.bufferSize(
        BandPassCorrectionKernel::BAND_PASS_CORRECTION_WEIGHTS)));

      bandPassCorrectionBuffers =
        std::auto_ptr<BandPassCorrectionKernel::Buffers>(
        new BandPassCorrectionKernel::Buffers(*devA, *devB,
        *devBandPassCorrectionWeights));

      bandPassCorrectionKernel = std::auto_ptr<BandPassCorrectionKernel>(
        factories.bandPassCorrection.create(queue, *bandPassCorrectionBuffers));

    }

    void BeamFormerPreprocessingStep::process(BlockID blockID,
      unsigned subband)
    {

      //****************************************
      // Enqueue the kernels
      // Note: make sure to call the right enqueue() for each kernel.
      // Otherwise, a kernel arg may not be set...
      DUMPBUFFER(intToFloatBuffers.input, "intToFloatBuffers.input.dat");
      intToFloatKernel->enqueue(blockID);

      firstFFTShiftKernel->enqueue(blockID);
      DUMPBUFFER(firstFFTShiftBuffers.output, "firstFFTShiftBuffers.output.dat");

      firstFFT->enqueue(blockID);
      DUMPBUFFER(delayCompensationBuffers.input, "firstFFT.output.dat");

      delayCompensationKernel->enqueue(
        blockID,
        ps.settings.subbands[subband].centralFrequency,
        ps.settings.subbands[subband].SAP);
      DUMPBUFFER(delayCompensationBuffers.output, "delayCompensationBuffers.output.dat");

      secondFFTShiftKernel->enqueue(blockID);
      DUMPBUFFER(secondFFTShiftBuffers.output, "secondFFTShiftBuffers.output.dat");

      secondFFT->enqueue(blockID);
      //DUMPBUFFER(bandPassCorrectionBuffers.input, "secondFFT.output.dat");

      bandPassCorrectionKernel->enqueue(
        blockID);
    }


    void BeamFormerPreprocessingStep::printStats()
    {
      // Print the individual counter stats: mean and stDev
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc FirstStage GPU mean and stDev ****" << endl <<
        std::setw(20) << "(intToFloatKernel)" << intToFloatKernel->itsCounter.stats << endl <<
        //std::setw(20) << "(firstFFTShift)" << firstFFTShift.stats << endl <<
        std::setw(20) << "(firstFFT)" << firstFFT->itsCounter.stats << endl <<
        std::setw(20) << "(delayCompensationKernel)" << delayCompensationKernel->itsCounter.stats << endl <<
        //std::setw(20) << "(secondFFTShift)" << secondFFTShift.stats << endl <<
        std::setw(20) << "(secondFFT)" << secondFFT->itsCounter.stats << endl <<
        std::setw(20) << "(bandPassCorrectionKernel)" << bandPassCorrectionKernel->itsCounter.stats << endl);
    }

    void BeamFormerPreprocessingStep::logTime()
    {
      intToFloatKernel->itsCounter.logTime();
      firstFFT->itsCounter.logTime();
      delayCompensationKernel->itsCounter.logTime();
      secondFFT->itsCounter.logTime();
      bandPassCorrectionKernel->itsCounter.logTime();
    }
  }
}
