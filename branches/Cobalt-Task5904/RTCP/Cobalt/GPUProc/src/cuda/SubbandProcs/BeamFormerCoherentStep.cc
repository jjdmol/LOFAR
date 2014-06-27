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

#include "BeamFormerSubbandProc.h"
#include "BeamFormerCoherentStep.h"

#include <iomanip>

namespace LOFAR
{
  namespace Cobalt
  {

    BeamFormerCoherentStep::Factories::Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc) :
      beamFormer(BeamFormerKernel::Parameters(ps)),
      coherentTranspose(CoherentStokesTransposeKernel::Parameters(ps)),
      coherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
        ps.settings.beamFormer.maxNrCoherentTABsPerSAP(),
        ps.settings.beamFormer.nrHighResolutionChannels)),
      coherentFirFilter(
        ps.settings.beamFormer.coherentSettings.nrChannels > 1
        ? new KernelFactory<FIR_FilterKernel>(FIR_FilterKernel::Parameters(ps,
            ps.settings.beamFormer.maxNrCoherentTABsPerSAP(),
            false,
            nrSubbandsPerSubbandProc,
            ps.settings.beamFormer.coherentSettings.nrChannels,
            static_cast<float>(ps.settings.beamFormer.coherentSettings.nrChannels)))
        : NULL),
      coherentStokes(CoherentStokesKernel::Parameters(ps))
    {
    }
  
    BeamFormerCoherentStep::BeamFormerCoherentStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      Factories &factories,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB)
      :
      ProcessStep(parset, i_queue),
      coherentStokesPPF(factories.coherentFirFilter != NULL),
      devC(context, factories.beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)),
      devD(context, factories.beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)),
      outputCounter(context)
    {
      devA = i_devA;
      devB = i_devB;
      initMembers(context, factories);
    }

    void BeamFormerCoherentStep::initMembers(gpu::Context &context,
      Factories &factories)
    {
    beamFormerKernel = std::auto_ptr<BeamFormerKernel>(
      factories.beamFormer.create(queue, *devB, *devA));

    // transpose after beamforming: A -> C/D
    //
    // Output buffer: 
    // 1ch: C
    // PPF: D

    coherentTransposeKernel = std::auto_ptr<CoherentStokesTransposeKernel>(
      factories.coherentTranspose.create(
      queue, *devA, coherentStokesPPF ? devD : devC));

    const size_t nrSamples = ps.settings.beamFormer.maxNrCoherentTABsPerSAP() * NR_POLARIZATIONS * ps.settings.blockSize;

    // inverse FFT: C/D -> C/D (in-place)
    inverseFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
      queue, ps.settings.beamFormer.nrHighResolutionChannels,
      nrSamples, false, coherentStokesPPF ? devD : devC));

    // fftshift: C/D -> C/D (in-place)
    inverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
      factories.coherentInverseFFTShift.create(
        queue,
        coherentStokesPPF ? devD : devC,
        coherentStokesPPF ? devD : devC));

    // FIR filter: D -> C
    //
    // Input buffer:
    // 1ch: - (no FIR will be done)
    // PPF: D
    //
    // Output buffer:
    // 1ch: - (no FIR will be done)
    // PPF: C
    if (coherentStokesPPF) {
      firFilterKernel = std::auto_ptr<FIR_FilterKernel>(
        factories.coherentFirFilter->create(queue, devD, devC));

      // final FFT: C -> C (in-place) = firFilterBuffers.output
      finalFFT = std::auto_ptr<FFT_Kernel>(new FFT_Kernel(
        queue, ps.settings.beamFormer.coherentSettings.nrChannels,
        nrSamples, true, devC));
    }

    // coherentStokes: C -> D
    //
    // 1ch: input comes from inverseFFT in C
    // Nch: input comes from finalFFT in C
    coherentStokesKernel = std::auto_ptr<CoherentStokesKernel>(
      factories.coherentStokes.create(queue, devC, devD));
  }

void BeamFormerCoherentStep::logTime()
{
  if (coherentStokesPPF) {
    firFilterKernel->itsCounter.logTime();
    finalFFT->itsCounter.logTime();
  }

  beamFormerKernel->itsCounter.logTime();
  coherentTransposeKernel->itsCounter.logTime();
  inverseFFT->itsCounter.logTime();
  coherentStokesKernel->itsCounter.logTime();
  outputCounter.logTime();
}


void BeamFormerCoherentStep::printStats()
{
  // Print the individual counter stats: mean and stDev
  LOG_INFO_STR(
    "**** BeamFormerSubbandProc coherent stage GPU mean and stDev ****" << endl <<
    std::setw(20) << "(firFilterKernel)" << (coherentStokesPPF ? firFilterKernel->itsCounter.stats : RunningStatistics()) << endl <<
    std::setw(20) << "(finalFFT)" << (coherentStokesPPF ? finalFFT->itsCounter.stats : RunningStatistics()) << endl <<
    std::setw(20) << "(beamformer)" << beamFormerKernel->itsCounter.stats << endl <<
    std::setw(20) << "(coherentTranspose)" << coherentTransposeKernel->itsCounter.stats << endl <<
    std::setw(20) << "(inverseFFT)" << inverseFFT->itsCounter.stats << endl <<
    std::setw(20) << "(inverseFFTShift)" << inverseFFTShiftKernel->itsCounter.stats << endl <<
    std::setw(20) << "(coherentStokes)" << coherentStokesKernel->itsCounter.stats << endl <<
    std::setw(20) << "(output)"         << outputCounter.stats << endl);

}


size_t BeamFormerCoherentStep::nrCoherent(const BlockID &blockID) const
{
  unsigned SAP = ps.settings.subbands[blockID.globalSubbandIdx].SAP;

  return ps.settings.beamFormer.SAPs[SAP].nrCoherent;
}


void BeamFormerCoherentStep::writeInput(const SubbandProcInputData &input)
{
  if (nrCoherent(input.blockID) == 0)
    return;

  // Upload the new beamformerDelays (pointings) to the GPU 
  queue.writeBuffer(beamFormerKernel->beamFormerDelays, input.tabDelays, false);
}


void BeamFormerCoherentStep::process(const SubbandProcInputData &input)
{
  if (nrCoherent(input.blockID) == 0)
    return;

  // The centralFrequency and SAP immediate kernel args must outlive kernel runs.
  beamFormerKernel->enqueue(input.blockID,
    ps.settings.subbands[input.blockID.globalSubbandIdx].centralFrequency,
    ps.settings.subbands[input.blockID.globalSubbandIdx].SAP);

  coherentTransposeKernel->enqueue(input.blockID);

  inverseFFT->enqueue(input.blockID);

  inverseFFTShiftKernel->enqueue(input.blockID);

  if (coherentStokesPPF) {
    // The subbandIdx immediate kernel arg must outlive kernel runs.
    firFilterKernel->enqueue(input.blockID,
      input.blockID.subbandProcSubbandIdx);
    finalFFT->enqueue(input.blockID);
  }

  coherentStokesKernel->enqueue(input.blockID);
}


void BeamFormerCoherentStep::readOutput(BeamFormedData &output)
{
  if (nrCoherent(output.blockID) == 0)
    return;

  output.coherentData.resizeOneDimensionInplace(0, nrCoherent(output.blockID));
  queue.readBuffer(output.coherentData, devD, outputCounter, false);
}


  }
}
