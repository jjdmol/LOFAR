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

#include "BeamFormerSubbandProcStep.h"
#include "BeamFormerIncoherentStep.h"
#include "BeamFormerFactories.h"

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

    BeamFormerIncoherentStep::BeamFormerIncoherentStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB,
      boost::shared_ptr<gpu::DeviceMemory> i_devC,
      boost::shared_ptr<gpu::DeviceMemory> i_devD,
      boost::shared_ptr<gpu::DeviceMemory> i_devE,

      boost::shared_ptr<gpu::DeviceMemory> i_devNull)
      :
      BeamFormerSubbandProcStep(parset, i_queue),
      outputComplexVoltages(ps.settings.beamFormer.coherentSettings.type == STOKES_XXYY),
      coherentStokesPPF(ps.settings.beamFormer.coherentSettings.nrChannels > 1)
    {
      devInput = i_devInput;
      devA = i_devA;
      devB = i_devB;
      devC = i_devC;
      devD = i_devD;
      devE = i_devE;
      devNull = i_devNull;
    }

    BeamFormerIncoherentStep::~BeamFormerIncoherentStep()
    {}

    void BeamFormerIncoherentStep::initMembers(gpu::Context &context,
      BeamFormerFactories &factories)
    {
      //incoherent stokes
      incoherentStokesPPF =
        ps.settings.beamFormer.incoherentSettings.nrChannels > 1;
      // Transpose: B -> A

      incoherentTransposeBuffers =
        std::auto_ptr<IncoherentStokesTransposeKernel::Buffers>(
        new IncoherentStokesTransposeKernel::Buffers(*devB, *devA));

      incoherentTranspose = std::auto_ptr<IncoherentStokesTransposeKernel>(
        factories.incoherentStokesTranspose.create(queue,
        *incoherentTransposeBuffers));

      unsigned incInversNrFFTs = ps.nrStations() * NR_POLARIZATIONS *
        ps.nrSamplesPerSubband() /
        ps.settings.beamFormer.nrHighResolutionChannels;

      incoherentInverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
        queue, ps.settings.beamFormer.nrHighResolutionChannels,
        incInversNrFFTs, false, *devA));

      // inverse FFTShift: A -> A
      incoherentInverseFFTShiftBuffers =
        std::auto_ptr<FFTShiftKernel::Buffers>(
        new FFTShiftKernel::Buffers(*devA, *devA));

      incoherentInverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
        factories.fftShift.create(queue, *incoherentInverseFFTShiftBuffers));

      devIncoherentFilterHistoryData = std::auto_ptr<gpu::DeviceMemory>(
        new gpu::DeviceMemory(context,
        factories.incoherentFirFilter.bufferSize(
        FIR_FilterKernel::HISTORY_DATA)
        ));
      devIncoherentFilterWeights = std::auto_ptr<gpu::DeviceMemory>(
        new gpu::DeviceMemory(context,
        factories.incoherentFirFilter.bufferSize(
        FIR_FilterKernel::FILTER_WEIGHTS)));

      incoherentFirFilterBuffers =
        std::auto_ptr<FIR_FilterKernel::Buffers>(
        new FIR_FilterKernel::Buffers(*devA, *devB,
        *devIncoherentFilterWeights,
        *devIncoherentFilterHistoryData));

      incoherentFirFilterKernel = std::auto_ptr<FIR_FilterKernel>(
        factories.incoherentFirFilter.create(
        queue, *incoherentFirFilterBuffers));


      // final FFT: B -> B
      unsigned nrFFTs = ps.nrStations() * NR_POLARIZATIONS *
        ps.nrSamplesPerSubband() /
        ps.settings.beamFormer.incoherentSettings.nrChannels;

      incoherentFinalFFT = std::auto_ptr<FFT_Kernel>(
        new FFT_Kernel(
        queue, ps.settings.beamFormer.incoherentSettings.nrChannels,
        nrFFTs, true, *devB));

      // incoherentstokes kernel: A/B -> E
      //
      // 1ch: input comes from incoherentInverseFFT in A
      // Nch: input comes from incoherentFinalFFT in B
      incoherentStokesBuffers =
        std::auto_ptr<IncoherentStokesKernel::Buffers>(
        new IncoherentStokesKernel::Buffers(
        incoherentStokesPPF ? *devB : *devA, *devE));
      incoherentStokesKernel = std::auto_ptr<IncoherentStokesKernel>(
        factories.incoherentStokes.create(queue, *incoherentStokesBuffers));

      devIncoherentFilterHistoryData->set(0);
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
      //incoherentOutput.logTime();  //transfer
    }


    void BeamFormerIncoherentStep::printStats()
    {
      LOG_INFO_STR(
        "**** BeamFormerSubbandProc incoherent stage GPU mean and stDev ****" << endl <<
        std::setw(20) << "(incoherentStokesTranspose)" << incoherentTranspose->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentInverseFFT)" << incoherentInverseFFT->itsCounter.stats << endl <<
        // std::setw(20) << "(incoherentInverseFFTShift)" << incoherentInverseFFTShift.stats << endl <<
        std::setw(20) << "(incoherentFirFilterKernel)" << incoherentFirFilterKernel->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentFinalFFT)" << incoherentFinalFFT->itsCounter.stats << endl <<
        std::setw(20) << "(incoherentStokes)" << incoherentStokesKernel->itsCounter.stats << endl);

    }


    void BeamFormerIncoherentStep::process(BlockID blockID,
      unsigned subband)
    {
      // ********************************************************************
      // incoherent stokes kernels
      incoherentTranspose->enqueue( blockID);

      incoherentInverseFFT->enqueue( blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.input,
        "incoherentInverseFFTShiftBuffers.input.dat");

      incoherentInverseFFTShiftKernel->enqueue( blockID);

      DUMPBUFFER(incoherentInverseFFTShiftBuffers.output,
        "incoherentInverseFFTShiftBuffers.output.dat");

      if (incoherentStokesPPF)
      {
        incoherentFirFilterKernel->enqueue(blockID,
          blockID.subbandProcSubbandIdx);

        incoherentFinalFFT->enqueue(blockID);
      }

      incoherentStokesKernel->enqueue( blockID);
    }
  }
}
