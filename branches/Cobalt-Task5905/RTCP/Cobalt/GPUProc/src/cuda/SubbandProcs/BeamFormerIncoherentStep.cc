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

#include "BeamFormerIncoherentStep.h"

#include <iomanip>

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerIncoherentStep::Factories::Factories(const Parset &ps, size_t nrSubbandsPerSubbandProc) :
      incoherentStokesTranspose(IncoherentStokesTransposeKernel::Parameters(ps)),

      incoherentInverseFFT(FFT_Kernel::Parameters(
        ps.settings.beamFormer.nrHighResolutionChannels,
        ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize, false,
        "FFT (incoherent, inverse)")),
      incoherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
        ps.settings.antennaFields.size(),
        ps.settings.beamFormer.nrHighResolutionChannels,
        "FFT-shift (incoherent, inverse)")),

      incoherentFirFilter(
        ps.settings.beamFormer.incoherentSettings.nrChannels > 1
        ? new KernelFactory<FIR_FilterKernel>(FIR_FilterKernel::Parameters(ps,
            ps.settings.antennaFields.size(),
            false,
            nrSubbandsPerSubbandProc,
            ps.settings.beamFormer.incoherentSettings.nrChannels,
            static_cast<float>(ps.settings.beamFormer.incoherentSettings.nrChannels),
            "FIR (incoherent, final)"))
        : NULL ),
      incoherentFinalFFT(
        ps.settings.beamFormer.incoherentSettings.nrChannels > 1
        ? new KernelFactory<FFT_Kernel>(FFT_Kernel::Parameters(
            ps.settings.beamFormer.incoherentSettings.nrChannels,
            ps.settings.antennaFields.size() * NR_POLARIZATIONS * ps.settings.blockSize, true,
            "FFT (incoherent, final)"))
        : NULL),

      incoherentStokes(IncoherentStokesKernel::Parameters(ps))
    {
    }

    BeamFormerIncoherentStep::BeamFormerIncoherentStep(
      const Parset &parset,
      gpu::Stream &i_queue,
      gpu::Context &context,
      Factories &factories,
      boost::shared_ptr<gpu::DeviceMemory> i_devA,
      boost::shared_ptr<gpu::DeviceMemory> i_devB )
      :
      ProcessStep(parset, i_queue),
      incoherentStokesPPF(factories.incoherentFirFilter != NULL),
      outputCounter(context, "output (incoherent)")
    {
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

      // inverse FFT: A -> A
      incoherentInverseFFT = std::auto_ptr<FFT_Kernel>(
        factories.incoherentInverseFFT.create(queue, *devA, *devA));

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
          factories.incoherentFinalFFT->create(queue, *devB, *devA));
      }

      // Incoherent Stokes kernel: A-> B
      //
      // 1ch: input comes from incoherentInverseFFT in A, output in B
      // Nch: input comes from incoherentFinalFFT in B, output in A
      incoherentStokesKernel = std::auto_ptr<IncoherentStokesKernel>(
        factories.incoherentStokes.create(
          queue, *devA, *devB));
    }

    gpu::DeviceMemory BeamFormerIncoherentStep::outputBuffer() {
      return *devB;
    }


    size_t BeamFormerIncoherentStep::nrIncoherent(const BlockID &blockID) const
    {
      unsigned SAP = ps.settings.subbands[blockID.globalSubbandIdx].SAP;

      return ps.settings.beamFormer.SAPs[SAP].nrIncoherent;
    }


    void BeamFormerIncoherentStep::process(const SubbandProcInputData &input)
    {
      if (nrIncoherent(input.blockID) == 0)
        return;

      // ********************************************************************
      // incoherent stokes kernels
      incoherentTranspose->enqueue(input.blockID);

      incoherentInverseFFT->enqueue(input.blockID);

      incoherentInverseFFTShiftKernel->enqueue(input.blockID);

      if (incoherentStokesPPF)
      {
        // The subbandIdx immediate kernel arg must outlive kernel runs.
        incoherentFirFilterKernel->enqueue(input.blockID,
          input.blockID.subbandProcSubbandIdx);

        incoherentFinalFFT->enqueue(input.blockID);
      }

      incoherentStokesKernel->enqueue(input.blockID);
    }


    void BeamFormerIncoherentStep::readOutput(BeamFormedData &output)
    {
      if (nrIncoherent(output.blockID) == 0)
        return;

      output.incoherentData.resizeOneDimensionInplace(0, nrIncoherent(output.blockID));
      queue.readBuffer(output.incoherentData, outputBuffer(), outputCounter, false);
    }
  }
}
