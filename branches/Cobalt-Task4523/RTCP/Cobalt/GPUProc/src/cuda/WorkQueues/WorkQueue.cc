//# WorkQueue.cc
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

#include "WorkQueue.h"

#include <Common/LofarLogger.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    WorkQueue::WorkQueue(const Parset &ps, gpu::Context &context)
    :
      ps(ps),
      queue(gpu::Stream(context))
    {
      // put enough objects in the inputPool to operate
      // TODO: Tweak the number of inputPool objects per WorkQueue,
      // probably something like max(3, nrSubbands/nrWorkQueues * 2), because
      // there both need to be enough items to receive all subbands at
      // once, and enough items to process the same amount in the
      // mean time.
      //
      // At least 3 items are needed for a smooth Pool operation.
      size_t nrInputDatas = std::max(3UL, ps.nrSubbands());
      for (size_t i = 0; i < nrInputDatas; ++i) {
        inputPool.free.append(new WorkQueueInputData(
                ps.nrBeams(),
                ps.nrStations(),
                NR_POLARIZATIONS,
                ps.nrHistorySamples() + ps.nrSamplesPerSubband(),
                ps.nrBytesPerComplexSample(),
                context));
      }
    }


    void WorkQueue::addCounter(const std::string &name)
    {
      counters[name] = new PerformanceCounter(name, profiling);
    }


    void WorkQueue::addTimer(const std::string &name)
    {
      timers[name] = new NSTimer(name, false, false);
    }


    // Get the log2 of the supplied number
    unsigned WorkQueue::Flagger::log2(unsigned n)
    {
      // Assure that the nrChannels is more then zero: never ending loop 
      ASSERT(powerOfTwo(n));

      unsigned log;
      for (log = 0; 1U << log != n; log ++)
        {;} // do nothing, the creation of the log is a side effect of the for loop

      //Alternative solution snipped:
      //int targetlevel = 0;
      //while (index >>= 1) ++targetlevel; 
      return log;
    }

    void WorkQueue::Flagger::convertFlagsToChannelFlags(Parset const &parset,
      MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
      MultiDimArray<SparseSet<unsigned>, 2>& flagsPerChannel)
    {
      unsigned numberOfChannels = parset.nrChannelsPerSubband();
      unsigned log2NrChannels = log2(numberOfChannels);
      //Convert the flags per sample to flags per channel
      for (unsigned station = 0; station < parset.nrStations(); station ++) 
      {
        // get the flag ranges
        const SparseSet<unsigned>::Ranges &ranges = inputFlags[station].getRanges();
        for (SparseSet<unsigned>::const_iterator it = ranges.begin();
          it != ranges.end(); it ++) 
        {
          unsigned begin_idx;
          unsigned end_idx;
          if (numberOfChannels == 1)
          {
            // do nothing, just take the ranges as supplied
            begin_idx = it->begin; 
            end_idx = std::min(parset.nrSamplesPerChannel(), it->end );
          }
          else
          {
            // Never flag before the start of the time range               
            // use bitshift to divide to the number of channels. 
            //
            // NR_TAPS is the width of the filter: they are
            // absorbed by the FIR and thus should be excluded
            // from the original flag set.
            //
            // At the same time, every sample is affected by
            // the NR_TAPS-1 samples before it. So, any flagged
            // sample in the input flags NR_TAPS samples in
            // the channel.
            begin_idx = std::max(0, 
              (signed) (it->begin >> log2NrChannels) - NR_TAPS + 1);

            // The min is needed, because flagging the last input
            // samples would cause NR_TAPS subsequent samples to
            // be flagged, which aren't necessarily part of this block.
            end_idx = std::min(parset.nrSamplesPerChannel() + 1, 
              ((it->end - 1) >> log2NrChannels) + 1);
          }

          // Now copy the transformed ranges to the channelflags
          for (unsigned ch = 0; ch < numberOfChannels; ch++) {
            flagsPerChannel[ch][station].include(begin_idx, end_idx);
          }
        }
      }
    }
  }
}

