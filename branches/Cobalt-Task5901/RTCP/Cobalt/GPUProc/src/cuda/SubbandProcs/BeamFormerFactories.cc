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
        fftShift(fftShiftParams(ps)),
        delayCompensation(delayCompensationParams(ps)),
        bandPassCorrection(bandPassCorrectionParams(ps)),

        beamFormer(beamFormerParams(ps)),
        coherentTranspose(coherentTransposeParams(ps)),
        coherentInverseFFTShift(coherentInverseFFTShiftParams(ps)),
        coherentFirFilter(coherentFirFilterParams(ps, nrSubbandsPerSubbandProc)),
        coherentStokes(coherentStokesParams(ps)),

        incoherentStokesTranspose(incoherentStokesTransposeParams(ps)),
        incoherentInverseFFTShift(incoherentInverseFFTShiftParams(ps)),
        incoherentFirFilter(
          incoherentFirFilterParams(ps, nrSubbandsPerSubbandProc)),
        incoherentStokes(incoherentStokesParams(ps))
      {
      }

      DelayAndBandPassKernel::Parameters
      BeamFormerFactories::delayCompensationParams(const Parset &ps) 
      {
        DelayAndBandPassKernel::Parameters params(ps, false);

        return params;
      }

      BandPassCorrectionKernel::Parameters
      BeamFormerFactories::bandPassCorrectionParams(const Parset &ps) 
      {
        BandPassCorrectionKernel::Parameters params(ps);

        return params;
      }

      BeamFormerKernel::Parameters 
      BeamFormerFactories::beamFormerParams(const Parset &ps) 
      {
        BeamFormerKernel::Parameters params(ps);

        return params;
      }

      CoherentStokesTransposeKernel::Parameters
      BeamFormerFactories::coherentTransposeParams(const Parset &ps) 
      {
        CoherentStokesTransposeKernel::Parameters params(ps);

        return params;
      }

      FFTShiftKernel::Parameters
      BeamFormerFactories::coherentInverseFFTShiftParams(const Parset &ps)
      {
        FFTShiftKernel::Parameters params(ps,
          std::max(1UL, ps.settings.beamFormer.maxNrCoherentTABsPerSAP()),
          ps.settings.beamFormer.nrHighResolutionChannels);

        return params;
      }

      FFTShiftKernel::Parameters
      BeamFormerFactories::incoherentInverseFFTShiftParams(const Parset &ps)
      {
        FFTShiftKernel::Parameters params(ps,
          ps.settings.antennaFields.size(),
          ps.settings.beamFormer.nrHighResolutionChannels);

        return params;
      }

      FIR_FilterKernel::Parameters
      BeamFormerFactories::
      coherentFirFilterParams(const Parset &ps,
                      size_t nrSubbandsPerSubbandProc) 
      {
        FIR_FilterKernel::Parameters params(ps,
          std::max(1UL, ps.settings.beamFormer.maxNrCoherentTABsPerSAP()),
          false,
          nrSubbandsPerSubbandProc,
          ps.settings.beamFormer.coherentSettings.nrChannels,
          static_cast<float>(ps.settings.beamFormer.coherentSettings.nrChannels));

        return params;
      }

      CoherentStokesKernel::Parameters
      BeamFormerFactories::coherentStokesParams(const Parset &ps) 
      {
        CoherentStokesKernel::Parameters params(ps);

        return params;
      }

      FFTShiftKernel::Parameters
      BeamFormerFactories::fftShiftParams(const Parset &ps) 
      {
        FFTShiftKernel::Parameters params(ps,
          ps.settings.antennaFields.size(),
          ps.settings.beamFormer.nrDelayCompensationChannels);

        return params;
      }

      FIR_FilterKernel::Parameters 
      BeamFormerFactories::
      incoherentFirFilterParams(const Parset &ps,
            size_t nrSubbandsPerSubbandProc)  
      {
        FIR_FilterKernel::Parameters params(ps,
          ps.settings.antennaFields.size(),
          false,
          nrSubbandsPerSubbandProc,
          ps.settings.beamFormer.incoherentSettings.nrChannels,
          static_cast<float>(ps.settings.beamFormer.incoherentSettings.nrChannels));

        return params;
      }

      IncoherentStokesKernel::Parameters 
      BeamFormerFactories::
      incoherentStokesParams(const Parset &ps)  
      {
        IncoherentStokesKernel::Parameters params(ps);

        return params;
      }

      IncoherentStokesTransposeKernel::Parameters 
      BeamFormerFactories::
      incoherentStokesTransposeParams(const Parset &ps)  
      {
        IncoherentStokesTransposeKernel::Parameters params(ps);

        return params;
      }
  }
}

