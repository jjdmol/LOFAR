//# BeamFormerCoherentStep.h
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

#include <complex>

#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>

#include "SubbandProc.h"

#include "BeamFormerIncoherentStep.h"

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
    BeamFormerIncoherentStep::Factories::Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc) :
      incoherentStokesTranspose(IncoherentStokesTransposeKernel::Parameters(ps)),
      incoherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
        ps.settings.antennaFields.size(),
        ps.settings.beamFormer.nrHighResolutionChannels)),
      incoherentFirFilter(
        ps.settings.beamFormer.incoherentSettings.nrChannels > 1
        ? new KernelFactory<FIR_FilterKernel>(FIR_FilterKernel::Parameters(ps,
            ps.settings.antennaFields.size(),
            false,
            nrSubbandsPerSubbandProc,
            ps.settings.beamFormer.incoherentSettings.nrChannels,
            static_cast<float>(ps.settings.beamFormer.incoherentSettings.nrChannels)))
        : NULL ),
      incoherentStokes(IncoherentStokesKernel::Parameters(ps))
    {
    }

    BeamFormerIncoherentStep::BeamFormerIncoherentStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      Factories &factories,
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB )
      :
      ProcessStep(parset, i_queue),
      incoherentStokesPPF(factories.incoherentFirFilter != NULL)
    {
      devInput = i_devInput;
      devA = i_devA;
      devB = i_devB;
      initMembers(context,
        factories);
    }


    void BeamFormerIncoherentStep::initMembers(gpu::Context &context,
      Factories &factories)
    {

      // Transpose: B -> A
      incoherentTranspose = std::auto_ptr<IncoherentStokesTransposeKernel>(
        factories.incoherentStokesTranspose.create(queue,
        *devB, *devA));

      const size_t nrSamples = ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize;

      // inverse FFT: A -> A
      incoherentInverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
        queue, ps.settings.beamFormer.nrHighResolutionChannels,
        nrSamples, false, *devA));

      // inverse FFTShift: A -> A
      incoherentInverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
        factories.incoherentInverseFFTShift.create(queue, *devA, *devA));

      if (incoherentStokesPPF) {
        // final FIR: A -> B
        incoherentFirFilterKernel = std::auto_ptr<FIR_FilterKernel>(
          factories.incoherentFirFilter->create(
          queue, *devA, *devB));

        // final FFT: B -> B
        incoherentFinalFFT = std::auto_ptr<FFT_Kernel>(
          new FFT_Kernel(
          queue, ps.settings.beamFormer.incoherentSettings.nrChannels,
          nrSamples, true, *devB));
      }

      // Incoherent Stokes kernel: A/B -> B/A
      //
      // 1ch: input comes from incoherentInverseFFT in A, output in B
      // Nch: input comes from incoherentFinalFFT in B, output in A
      incoherentStokesKernel = std::auto_ptr<IncoherentStokesKernel>(
        factories.incoherentStokes.create(
          queue,
          incoherentStokesPPF ? *devB : *devA,
          incoherentStokesPPF ? *devA : *devB));
    }

    gpu::DeviceMemory BeamFormerIncoherentStep::outputBuffer() {
      return incoherentStokesPPF ? *devA : *devB;
    }

    void BeamFormerIncoherentStep::logTime()
    {
      incoherentTranspose->itsCounter.logTime();
      incoherentInverseFFT->itsCounter.logTime();
      if (incoherentStokesPPF)
      {
        incoherentFirFilterKernel->itsCounter.logTime();
        incoherentFinalFFT->itsCounter.logTime();
      }
      incoherentStokesKernel->itsCounter.logTime();
    }

    void BeamFormerIncoherentStep::printStats()
    {
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc incoherent stage GPU mean and stDev ****" << endl <<
        std::setw(20) << "(incoherentStokesTranspose)" << incoherentTranspose->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentInverseFFTShift)" << incoherentInverseFFTShiftKernel->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentFirFilterKernel)" << (incoherentStokesPPF ? incoherentFirFilterKernel->itsCounter.stats : RunningStatistics()) << endl <<
        std::setw(20) << "(incoherentFinalFFT)" << (incoherentStokesPPF ? incoherentFinalFFT->itsCounter.stats : RunningStatistics()) << endl <<
        std::setw(20) << "(incoherentStokes)" << incoherentStokesKernel->itsCounter.stats << endl);
    }

    void BeamFormerIncoherentStep::process(BlockID blockID, unsigned /*subband*/)
    {
      // ********************************************************************
      // incoherent stokes kernels
      incoherentTranspose->enqueue(blockID);

      incoherentInverseFFT->enqueue(blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.input,
        "incoherentInverseFFTShiftBuffers.input.dat");

      incoherentInverseFFTShiftKernel->enqueue(blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.output,
        "incoherentInverseFFTShiftBuffers.output.dat");

      if (incoherentStokesPPF)
      {
        // The subbandIdx immediate kernel arg must outlive kernel runs.
        incoherentFirFilterKernel->enqueue(blockID,
          blockID.subbandProcSubbandIdx);

        incoherentFinalFFT->enqueue(blockID);
      }

      incoherentStokesKernel->enqueue(blockID);
    }
  }
}
