//# ProcessStep.h
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

#ifndef LOFAR_GPUPROC_CUDA_PROCESS_STEP_H
#define LOFAR_GPUPROC_CUDA_PROCESS_STEP_H

#include <CoInterface/Parset.h>

#include <GPUProc/gpu_wrapper.h>

#include <GPUProc/SubbandProcs/SubbandProc.h>

#include "SubbandProc.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class ProcessStep
    {
    public:
      virtual void process(const SubbandProcInputData &input) = 0;

      virtual void printStats() =0;

      virtual void logTime() = 0;

      virtual ~ProcessStep()
       {}

    protected:
      ProcessStep(const Parset &parset,
        gpu::Stream &queue)
        :
        ps(parset),
        queue(queue) 
        {};

      const Parset ps;
      gpu::Stream queue;
    };
  }
}

#endif
