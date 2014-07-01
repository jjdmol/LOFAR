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

      coherentInverseFFT(FFT_Kernel::Parameters(
        ps.settings.beamFormer.nrHighResolutionChannels,
        ps.settings.beamFormer.maxNrCoherentTABsPerSAP() * NR_POLARIZATIONS * ps.settings.blockSize,
        false,
        "FFT (coherent, inverse)")),
      coherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
        ps.settings.beamFormer.maxNrCoherentTABsPerSAP(),
        ps.settings.beamFormer.nrHighResolutionChannels,
        "FFT-shift (coherent, inverse)")),

      coherentFirFilter(
        ps.settings.beamFormer.coherentSettings.nrChannels > 1
        ? new KernelFactory<FIR_FilterKernel>(FIR_FilterKernel::Parameters(ps,
            ps.settings.beamFormer.maxNrCoherentTABsPerSAP(),
            false,
            nrSubbandsPerSubbandProc,
            ps.settings.beamFormer.coherentSettings.nrChannels,
            static_cast<float>(ps.settings.beamFormer.coherentSettings.nrChannels),
            "FIR (coherent, final)"))
        : NULL),
      coherentFinalFFT(
        ps.settings.beamFormer.coherentSettings.nrChannels > 1
        ? new KernelFactory<FFT_Kernel>(FFT_Kernel::Parameters(
            ps.settings.beamFormer.coherentSettings.nrChannels,
            ps.settings.beamFormer.maxNrCoherentTABsPerSAP() * NR_POLARIZATIONS * ps.settings.blockSize,
            true,
            "FFT (coherent, final)"))
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
      outputCounter(context, "output (coherent)")
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

    // transpose after beamforming: A -> C
    coherentTransposeKernel = std::auto_ptr<CoherentStokesTransposeKernel>(
      factories.coherentTranspose.create(
      queue, *devA, devC));

    // inverse FFT: C -> C (in-place)
    inverseFFT = std::auto_ptr<FFT_Kernel>(
      factories.coherentInverseFFT.create(
        queue, devC, devC));

    // fftshift: C -> C (in-place)
    inverseFFTShiftKernel = std::auto_ptr<FFTShiftKernel>(
      factories.coherentInverseFFTShift.create(
        queue, devC, devC));

    if (coherentStokesPPF) {
      // FIR filter: C -> D
      firFilterKernel = std::auto_ptr<FIR_FilterKernel>(
        factories.coherentFirFilter->create(queue, devC, devD));

      // final FFT: D -> D (in-place) = firFilterBuffers.output
      coherentFinalFFT = std::auto_ptr<FFT_Kernel>(
        factories.coherentFinalFFT->create(queue, devD, devD));
    }

    // coherentStokes:
    //    coherentstokesPPF: D -> C
    //                 else: C -> D
    coherentStokesKernel = std::auto_ptr<CoherentStokesKernel>(
      factories.coherentStokes.create(queue,
      coherentStokesPPF ? devD : devC,
      coherentStokesPPF ? devC : devD));
  }

  gpu::DeviceMemory BeamFormerCoherentStep::outputBuffer() {
    return coherentStokesPPF ? devC : devD;
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
    coherentFinalFFT->enqueue(input.blockID);
  }

  coherentStokesKernel->enqueue(input.blockID);
}


void BeamFormerCoherentStep::readOutput(BeamFormedData &output)
{
  if (nrCoherent(output.blockID) == 0)
    return;

  output.coherentData.resizeOneDimensionInplace(0, nrCoherent(output.blockID));
  queue.readBuffer(output.coherentData, outputBuffer(), outputCounter, false);
}


  }
}
