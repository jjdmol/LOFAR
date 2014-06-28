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

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_wrapper.h>

#include <CoInterface/Parset.h>
#include <ApplCommon/PosixTime.h>
#include <Common/LofarLogger.h>

#include <iomanip>

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerPreprocessingStep::Factories::Factories(const Parset &ps) :
        intToFloat(ps),
        fftShift(FFTShiftKernel::Parameters(ps,
          ps.settings.antennaFields.size(),
          ps.settings.beamFormer.nrDelayCompensationChannels)),
        delayCompensation(DelayAndBandPassKernel::Parameters(ps, false)),
        bandPassCorrection(BandPassCorrectionKernel::Parameters(ps))
    {
    }

    BeamFormerPreprocessingStep::BeamFormerPreprocessingStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      Factories &factories,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB)
      :
      ProcessStep(parset, i_queue)
    {
      devA=i_devA;
      devB=i_devB;
      initMembers(context, factories);
    }

    BeamFormerPreprocessingStep::~BeamFormerPreprocessingStep()
    {}

    void BeamFormerPreprocessingStep::initMembers(gpu::Context &context,
      Factories &factories){

      doSecondFFT = 
        (ps.settings.beamFormer.nrHighResolutionChannels /
         ps.settings.beamFormer.nrDelayCompensationChannels) > 1;

      intToFloatKernel = std::auto_ptr<IntToFloatKernel>(
        factories.intToFloat.create(queue, *devA, *devB));

      // FFTShift: B -> B
      firstFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
        factories.fftShift.create(queue, *devB, *devB));

      const size_t nrSamples = ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize;

      // FFT: B -> B
      firstFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(queue,
        ps.settings.beamFormer.nrDelayCompensationChannels,
        nrSamples, true, *devB));

      // delayComp: B -> A
      delayCompensationKernel = std::auto_ptr<DelayAndBandPassKernel>(
        factories.delayCompensation.create(queue, *devB, *devA));


      // Only perform second FFTshift and FFT if we have to.
      if (doSecondFFT) {

        // FFTShift: A -> A
        secondFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
          factories.fftShift.create(queue, *devA, *devA));

        // FFT: A -> A
        secondFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(queue,
          ps.settings.beamFormer.nrHighResolutionChannels /
          ps.settings.beamFormer.nrDelayCompensationChannels,
          nrSamples, true, *devA));
      }

      // bandPass: A -> B
      bandPassCorrectionKernel = std::auto_ptr<BandPassCorrectionKernel>(
        factories.bandPassCorrection.create(queue, *devA, *devB));

    }

    void BeamFormerPreprocessingStep::writeInput(const SubbandProcInputData &input)
    {
      if (ps.settings.delayCompensation.enabled)
      {
        queue.writeBuffer(delayCompensationKernel->delaysAtBegin,
          input.delaysAtBegin, false);
        queue.writeBuffer(delayCompensationKernel->delaysAfterEnd,
          input.delaysAfterEnd, false);
        queue.writeBuffer(delayCompensationKernel->phase0s,
          input.phase0s, false);
      }
    }

    void BeamFormerPreprocessingStep::process(const SubbandProcInputData &input)
    {

      //****************************************
      // Enqueue the kernels
      // Note: make sure to call the right enqueue() for each kernel.
      // Otherwise, a kernel arg may not be set...
      intToFloatKernel->enqueue(input.blockID);

      firstFFTShiftKernel->enqueue(input.blockID);

      firstFFT->enqueue(input.blockID);

      // The centralFrequency and SAP immediate kernel args must outlive kernel runs.
      delayCompensationKernel->enqueue(
        input.blockID,
        ps.settings.subbands[input.blockID.globalSubbandIdx].centralFrequency,
        ps.settings.subbands[input.blockID.globalSubbandIdx].SAP);

      if (doSecondFFT) {
        secondFFTShiftKernel->enqueue(input.blockID);

        secondFFT->enqueue(input.blockID);
      }

      bandPassCorrectionKernel->enqueue(
        input.blockID);
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
        std::setw(20) << "(secondFFT)" << 
        (doSecondFFT ? secondFFT->itsCounter.stats : RunningStatistics()) << endl <<
        std::setw(20) << "(bandPassCorrectionKernel)" << bandPassCorrectionKernel->itsCounter.stats << endl);
    }

    void BeamFormerPreprocessingStep::logTime()
    {
      intToFloatKernel->itsCounter.logTime();
      firstFFT->itsCounter.logTime();
      delayCompensationKernel->itsCounter.logTime();
      if (doSecondFFT) secondFFT->itsCounter.logTime();
      bandPassCorrectionKernel->itsCounter.logTime();
    }
  }
}
