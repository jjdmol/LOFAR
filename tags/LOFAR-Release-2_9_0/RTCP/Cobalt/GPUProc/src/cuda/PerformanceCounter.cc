//# PerformanceCounter.cc
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

#include <iomanip>

#include "PerformanceCounter.h"
#include <Common/LofarLogger.h>
#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * The performance is measured by posting start and stop events into the context.
     *
     * The caller calls recordStart() and recordStop() to do so. The time span between
     * events can be measured only after they have occurred (after the stream synchronises with
     * the CPU). To not depend on stream synchronisation here, we simply query the time between
     * stop and start (logTime()) when we
     *   a) start a new measurement (recordStart), or
     *   b) on destruction
     */
    PerformanceCounter::PerformanceCounter(const gpu::Context &context, const std::string &name)
      :
    name(name),
    start(context),
    stop(context),
    recording(false)
    {}

    PerformanceCounter::~PerformanceCounter()
    {
      if (!gpuProfiling)
        return;

      // record any lingering information
      logTime();

      LOG_INFO_STR("(" << std::setw(30) << name << "): " << stats);
    }


    void PerformanceCounter::recordStart(const gpu::Stream &stream)
    {
      if (!gpuProfiling)
        return;

      // record any lingering information
      logTime();

      stream.recordEvent(start);
      recording = true;
    }


    void PerformanceCounter::recordStop(const gpu::Stream &stream)
    {
      if (!gpuProfiling)
        return;

      stream.recordEvent(stop);
    }



    void PerformanceCounter::logTime()
    {
      if (!recording)
        return;

      recording = false;

      // get the difference between start and stop. push it on the stats object
      try {
        stats.push(stop.elapsedTime(start));
      } catch (LOFAR::Cobalt::gpu::CUDAException) {
        // catch errors in case the event was not posted -- the current interface
        // has no easy way to check beforehand.
      }
    }
    
  }
}

