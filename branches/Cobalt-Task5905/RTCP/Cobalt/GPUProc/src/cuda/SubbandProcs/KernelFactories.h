//# KernelFactories.h
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

#ifndef LOFAR_GPUPROC_CUDA_BEAM_FORMER_FACTORIES_H
#define LOFAR_GPUPROC_CUDA_BEAM_FORMER_FACTORIES_H

#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>

#include "CorrelatorStep.h"
#include "BeamFormerPreprocessingStep.h"
#include "BeamFormerCoherentStep.h"
#include "BeamFormerIncoherentStep.h"

namespace LOFAR
{
  namespace Cobalt
  {
    struct KernelFactories
    {
      KernelFactories(const Parset &ps, 
                            size_t nrSubbandsPerSubbandProc = 1);

      SmartPtr<CorrelatorStep::Factories> correlator;

      SmartPtr<BeamFormerPreprocessingStep::Factories> preprocessing;

      SmartPtr<BeamFormerCoherentStep::Factories> coherentStokes;
      SmartPtr<BeamFormerIncoherentStep::Factories> incoherentStokes;
    };

  }
}

#endif
