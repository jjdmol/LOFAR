//# KernelFactories.cc
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

#include "KernelFactories.h"

namespace LOFAR
{
  namespace Cobalt
  {
    KernelFactories::KernelFactories(const Parset &ps,
                                             size_t nrSubbandsPerSubbandProc) :
      correlator(ps.settings.correlator.enabled
        ? new CorrelatorStep::Factories(ps, nrSubbandsPerSubbandProc)
        : NULL),

      preprocessing(ps.settings.beamFormer.enabled
        ? new BeamFormerPreprocessingStep::Factories(ps)
        : NULL),

      coherentStokes(ps.settings.beamFormer.anyCoherentTABs()
        ? new BeamFormerCoherentStep::Factories(ps, nrSubbandsPerSubbandProc)
        : NULL),

      incoherentStokes(ps.settings.beamFormer.anyIncoherentTABs()
        ? new BeamFormerIncoherentStep::Factories(ps, nrSubbandsPerSubbandProc)
        : NULL)
    {
    }
  }
}

