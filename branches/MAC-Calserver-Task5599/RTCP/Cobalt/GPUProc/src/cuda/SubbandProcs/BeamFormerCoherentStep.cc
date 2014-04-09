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
#include "BeamFormerCoherentStep.h"
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
  
    BeamFormerCoherentStep::BeamFormerCoherentStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      BeamFormerFactories &factories,
      boost::shared_ptr<SubbandProcInputData::DeviceBuffers> i_devInput,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB,
      boost::shared_ptr<gpu::DeviceMemory> i_devC,
      boost::shared_ptr<gpu::DeviceMemory> i_devD,
      boost::shared_ptr<gpu::DeviceMemory> i_devBeamFormerDelays,
      boost::shared_ptr<gpu::DeviceMemory> i_devNull)
      :
      BeamFormerSubbandProcStep(parset, i_queue),
      coherentStokesPPF(ps.settings.beamFormer.coherentSettings.nrChannels > 1)
    {
      devInput = i_devInput;
      devA = i_devA;
      devB = i_devB;
      devC = i_devC;
      devD = i_devD;
      devBeamFormerDelays = i_devBeamFormerDelays;
      devNull = i_devNull;
      initMembers(context, factories);
    }

    BeamFormerCoherentStep::~BeamFormerCoherentStep()
    {}

    void BeamFormerCoherentStep::initMembers(gpu::Context &context,
      BeamFormerFactories &factories)
    {
    beamFormerBuffers = std::auto_ptr<BeamFormerKernel::Buffers>(
      new BeamFormerKernel::Buffers(*devB, *devA, *devBeamFormerDelays));

    beamFormerKernel = std::auto_ptr<BeamFormerKernel>(
      factories.beamFormer.create(queue, *beamFormerBuffers));

    // transpose after beamforming: A -> C/D
    //
    // Output buffer: 
    // 1ch: C
    // PPF: D

    coherentTransposeBuffers = std::auto_ptr<CoherentStokesTransposeKernel::Buffers>(
      new CoherentStokesTransposeKernel::Buffers(*devA, coherentStokesPPF ? *devD : *devC));

    coherentTransposeKernel = std::auto_ptr<CoherentStokesTransposeKernel>(
      factories.coherentTranspose.create(
      queue, *coherentTransposeBuffers));

    // inverse FFT: C/D -> C/D (in-place)
    unsigned nrInverFFTs = ps.settings.beamFormer.maxNrTABsPerSAP() *
      NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
      ps.settings.beamFormer.nrHighResolutionChannels;
    inverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
      queue, ps.settings.beamFormer.nrHighResolutionChannels,
      nrInverFFTs, false, coherentStokesPPF ? *devD : *devC));

    // fftshift: C/D -> C/D (in-place)
    inverseFFTShiftBuffers = std::auto_ptr<FFTShiftKernel::Buffers>(
      new FFTShiftKernel::Buffers(coherentStokesPPF ? *devD : *devC, coherentStokesPPF ? *devD : *devC));

    inverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
      factories.fftShift.create(queue, *inverseFFTShiftBuffers));

    // FIR filter: D -> C
    //
    // Input buffer:
    // 1ch: - (no FIR will be done)
    // PPF: D
    //
    // Output buffer:
    // 1ch: - (no FIR will be done)
    // PPF: C
    devFilterWeights = std::auto_ptr<gpu::DeviceMemory>(
      new gpu::DeviceMemory(context,
      factories.firFilter.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)));

    devFilterHistoryData = std::auto_ptr<gpu::DeviceMemory>(
      new gpu::DeviceMemory(context,
      factories.firFilter.bufferSize(FIR_FilterKernel::HISTORY_DATA)));

    firFilterBuffers = std::auto_ptr<FIR_FilterKernel::Buffers>(
      new FIR_FilterKernel::Buffers(*devD, *devC, *devFilterWeights, *devFilterHistoryData));

    firFilterKernel = std::auto_ptr<FIR_FilterKernel>(
      factories.firFilter.create(queue, *firFilterBuffers));

    // final FFT: C -> C (in-place) = firFilterBuffers.output

    unsigned nrFinalFFTs = ps.settings.beamFormer.maxNrTABsPerSAP() *
      NR_POLARIZATIONS * ps.nrSamplesPerSubband() /
      ps.settings.beamFormer.coherentSettings.nrChannels;
    finalFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
      queue, ps.settings.beamFormer.coherentSettings.nrChannels,
      nrFinalFFTs, true, *devC));

    // coherentStokes: C -> D
    //
    // 1ch: input comes from inverseFFT in C
    // Nch: input comes from finalFFT in C

    coherentStokesBuffers = std::auto_ptr<CoherentStokesKernel::Buffers>(
      new CoherentStokesKernel::Buffers(*devC, *devD));

    coherentStokesKernel = std::auto_ptr<CoherentStokesKernel>(
      factories.coherentStokes.create(queue, *coherentStokesBuffers));

    // initialize history data for both coherent and incoherent stokes.
    devFilterHistoryData->set(0);
  }

void BeamFormerCoherentStep::logTime()
{
  if (coherentStokesPPF)
  {
    firFilterKernel->itsCounter.logTime();
    finalFFT->itsCounter.logTime();
  }

  beamFormerKernel->itsCounter.logTime();
  coherentTransposeKernel->itsCounter.logTime();
  inverseFFT->itsCounter.logTime();
  coherentStokesKernel->itsCounter.logTime();
  //visibilities.logTime(); //transfer

}


void BeamFormerCoherentStep::printStats()
{
  // Print the individual counter stats: mean and stDev
  LOG_INFO_STR(
    "**** BeamFormerSubbandProc coherent stage GPU mean and stDev ****" << endl <<
    std::setw(20) << "(firFilterKernel)" << firFilterKernel->itsCounter.stats << endl <<
    std::setw(20) << "(finalFFT)" << finalFFT->itsCounter.stats << endl <<
    std::setw(20) << "(beamformer)" << beamFormerKernel->itsCounter.stats << endl <<
    std::setw(20) << "(coherentTranspose)" << coherentTransposeKernel->itsCounter.stats << endl <<
    std::setw(20) << "(inverseFFT)" << inverseFFT->itsCounter.stats << endl <<
    //std::setw(20) << "(inverseFFTShift)" << inverseFFTShift.stats << endl <<
    std::setw(20) << "(coherentStokes)" << coherentStokesKernel->itsCounter.stats << endl);

}


void BeamFormerCoherentStep::process(BlockID blockID,
  unsigned subband)
{
  beamFormerKernel->enqueue(blockID,
    ps.settings.subbands[subband].centralFrequency,
    ps.settings.subbands[subband].SAP);

  coherentTransposeKernel->enqueue(blockID);
  DUMPBUFFER(coherentTransposeBuffers.output, "coherentTransposeBuffers.output.dat");

  inverseFFT->enqueue(blockID);
  DUMPBUFFER(inverseFFTShiftBuffers.input, "inverseFFTBuffers.output.dat");

  inverseFFTShiftKernel->enqueue(blockID);
  DUMPBUFFER(inverseFFTShiftBuffers.output, "inverseFFTShift.output.dat");

  if (coherentStokesPPF)
  {
    firFilterKernel->enqueue(blockID,
      blockID.subbandProcSubbandIdx);
    finalFFT->enqueue(blockID);
  }

  DUMPBUFFER(coherentStokesBuffers.input, "coherentStokesBuffers.input.dat");
  coherentStokesKernel->enqueue(blockID);
}


  }
}
