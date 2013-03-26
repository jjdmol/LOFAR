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

#include <string>
#include <map>

#include <Common/Timer.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>

#include <opencl-incl.h>
#include <Pipeline.h>
#include <PerformanceCounter.h>
#include <FilterBank.h>
#include "CorrelatorPipelinePrograms.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class CorrelatorWorkQueue;

    class CorrelatorPipeline : public Pipeline
    {
    public:
      CorrelatorPipeline(const Parset &);

      void                    doWork();
      void        doWorkQueue(CorrelatorWorkQueue &workQueue);
      void        receiveSubbandSamples(CorrelatorWorkQueue &workQueue, unsigned subband);

    private:
      friend class CorrelatorWorkQueue;

      FilterBank filterBank;
      CorrelatorPipelinePrograms programs;

      struct Performance {
        std::map<std::string, PerformanceCounter::figures> total_counters;
        std::map<std::string, SmartPtr<NSTimer> > total_timers;
        Mutex totalsMutex;

        void addQueue(CorrelatorWorkQueue &queue);
        void log(size_t nrWorkQueues);
      } performance;

    };

  }
}

#endif

