//# Trigger.h: boolean condition to broadcast over threads
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id: Trigger.h 17975 2011-05-10 09:52:51Z mol $ 

#ifndef  LOFAR_LCS_COMMON_TRIGGER_H
#define  LOFAR_LCS_COMMON_TRIGGER_H

#ifdef USE_THREADS

#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>

 
namespace LOFAR {


class Trigger
{
  public:
    Trigger();

    void trigger();
    void wait();
    bool wait(const struct timespec &timespec);

    void noMore();
    
  private:
    Trigger(const Trigger&);
    Trigger& operator=(const Trigger&);

    Mutex     mutex;
    Condition condition;
    bool      triggered;
};


inline Trigger::Trigger()
:
  triggered(false)
{
}


inline void Trigger::trigger()
{
  ScopedLock lock(mutex);

  triggered = true;
  condition.broadcast();
}


inline void Trigger::wait()
{
  ScopedLock lock(mutex);

  while (!triggered)
    condition.wait(mutex);
}


inline bool Trigger::wait(const struct timespec &timespec)
{
  ScopedLock lock(mutex);

  while (!triggered)
    if (!condition.wait(mutex, timespec))
      return false;

  return true;
}

} // namespace LOFAR

#endif

#endif
