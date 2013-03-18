/* LockedRanges.h
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_GPUPROC_LOCKED_RANGES_H
#define LOFAR_GPUPROC_LOCKED_RANGES_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/LofarLogger.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/SparseSet.h>


namespace LOFAR
{
  namespace Cobalt
  {

    class LockedRanges
    {
    public:
      LockedRanges(unsigned bufferSize);

      void lock(unsigned begin, unsigned end);
      void unlock(unsigned begin, unsigned end);

    private:
      SparseSet<unsigned> itsLockedRanges;
      Mutex itsMutex;
      Condition itsRangeUnlocked;
      const unsigned itsBufferSize;
    };


    inline LockedRanges::LockedRanges(unsigned bufferSize)
      :
      itsBufferSize(bufferSize)
    {
    }


    inline void LockedRanges::lock(unsigned begin, unsigned end)
    {
      ScopedLock scopedLock(itsMutex);

      if (begin < end) {
        while (itsLockedRanges.subset(begin, end).count() > 0) {
          LOG_WARN_STR("Circular buffer: reader & writer try to use overlapping sections, range to lock = (" << begin << ", " << end << "), already locked = " << itsLockedRanges);
          itsRangeUnlocked.wait(itsMutex);
        }

        itsLockedRanges.include(begin, end);
      } else {
        while (itsLockedRanges.subset(begin, itsBufferSize).count() > 0 || itsLockedRanges.subset(0, end).count() > 0) {
          LOG_WARN_STR("Circular buffer: reader & writer try to use overlapping sections, range to lock = (" << begin << ", " << end << "), already locked = " << itsLockedRanges);
          itsRangeUnlocked.wait(itsMutex);
        }

        itsLockedRanges.include(begin, itsBufferSize).include(0, end);
      }
    }


    inline void LockedRanges::unlock(unsigned begin, unsigned end)
    {
      ScopedLock scopedLock(itsMutex);

      if (begin < end)
        itsLockedRanges.exclude(begin, end);
      else
        itsLockedRanges.exclude(end, itsBufferSize).exclude(0, begin);

      itsRangeUnlocked.broadcast();
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

