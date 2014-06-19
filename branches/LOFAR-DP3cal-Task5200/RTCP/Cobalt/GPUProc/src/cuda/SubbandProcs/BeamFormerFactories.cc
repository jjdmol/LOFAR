//# BeamFormerFactories.cc
//#
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

#include "BeamFormerFactories.h"
#include "BeamFormerSubbandProc.h"

namespace LOFAR
{
  namespace Cobalt
  {
    BeamFormerFactories::BeamFormerFactories(const Parset &ps,
                                             size_t nrSubbandsPerSubbandProc) :
        intToFloat(ps),
        fftShift(FFTShiftKernel::Parameters(ps,
          ps.settings.antennaFields.size(),
          ps.settings.beamFormer.nrDelayCompensationChannels)),
        delayCompensation(DelayAndBandPassKernel::Parameters(ps, false)),
        bandPassCorrection(BandPassCorrectionKernel::Parameters(ps)),
        beamFormer(BeamFormerKernel::Parameters(ps)),

        coherentTranspose(CoherentStokesTransposeKernel::Parameters(ps)),
        coherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
          std::max(1UL, ps.settings.beamFormer.maxNrCoherentTABsPerSAP()),
          ps.settings.beamFormer.nrHighResolutionChannels)),
        coherentFirFilter(FIR_FilterKernel::Parameters(ps,
          std::max(1UL, ps.settings.beamFormer.maxNrCoherentTABsPerSAP()),
          false,
          nrSubbandsPerSubbandProc,
          ps.settings.beamFormer.coherentSettings.nrChannels,
          static_cast<float>(ps.settings.beamFormer.coherentSettings.nrChannels))),
        coherentStokes(CoherentStokesKernel::Parameters(ps)),

        incoherentStokesTranspose(IncoherentStokesTransposeKernel::Parameters(ps)),
        incoherentInverseFFTShift(FFTShiftKernel::Parameters(ps,
          ps.settings.antennaFields.size(),
          ps.settings.beamFormer.nrHighResolutionChannels)),
        incoherentFirFilter(FIR_FilterKernel::Parameters(ps,
          ps.settings.antennaFields.size(),
          false,
          nrSubbandsPerSubbandProc,
          ps.settings.beamFormer.incoherentSettings.nrChannels,
          static_cast<float>(ps.settings.beamFormer.incoherentSettings.nrChannels))),
        incoherentStokes(IncoherentStokesKernel::Parameters(ps))
      {
      }
  }
}

