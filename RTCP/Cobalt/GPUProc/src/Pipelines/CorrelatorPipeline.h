//# CorrelatorPipeline.h
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

#ifndef LOFAR_GPUPROC_CORRELATOR_PIPELINE_H
#define LOFAR_GPUPROC_CORRELATOR_PIPELINE_H

#include <CoInterface/Parset.h>

#include <GPUProc/opencl-incl.h>
#include <GPUProc/Pipeline.h>
#include <GPUProc/FilterBank.h>
#include "CorrelatorPipelinePrograms.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class CorrelatorWorkQueue;

    // Correlator pipeline, connect input, GPUWorkQueues and output in a parallel (OMP).
    // Connect all parts of the pipeline together: set up connections with the input stream
    // each in a seperate thread. Start 2 WorkQueues for each GPU in the system.
    // The WorkQueue are then filled with data from the stream. And started to 
    // work. After all data is collected the output is written, again all parallel.
    // This class contains most CPU side parallelism.
    // It also contains two 'data' members that are shared between queues
    class CorrelatorPipeline : public Pipeline
    {
    public:
      CorrelatorPipeline(const Parset &);

      // per thread/station start up the input create 2 WorkQueue for each available GPU
      void        doWork();
      // for each subband get data from input stream, sync, start the kernels to process all data, write output in parallel
      void        doWorkQueue(CorrelatorWorkQueue &workQueue);
      // Read for a subband the data from the station steams, and put in shared memory
      void        receiveSubbandSamples(CorrelatorWorkQueue &workQueue, unsigned subband);

    private:
      FilterBank filterBank;
      CorrelatorPipelinePrograms programs;
    };
  }
}
#endif
