//# WorkQueue.h
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

#ifndef LOFAR_GPUPROC_CUDA_WORKQUEUE_H
#define LOFAR_GPUPROC_CUDA_WORKQUEUE_H

#include <string>
#include <map>

#include <Common/Timer.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class WorkQueue
    {
    public:
      WorkQueue(const Parset &ps, gpu::Context &context);

      // TODO: clean up access by Pipeline class and move under protected
      std::map<std::string, SmartPtr<PerformanceCounter> > counters;
      std::map<std::string, SmartPtr<NSTimer> > timers;

      class Flagger
      {
      public:
        // 1.1 Convert the flags per station to channel flags, change time scale if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 1> const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChannel);

        // 1.3 Get the LOG2 of the input. Used to speed up devisions by 2
        static unsigned log2(unsigned n);
      };

    protected:
      const Parset &ps;

      gpu::Stream queue;

      void addCounter(const std::string &name);
      void addTimer(const std::string &name);
    };
  }
}

#endif

