//# Semaphore.h: semaphore implementation on top of pthreads
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
//# $Id$ 

#ifndef  LOFAR_LCS_THREAD_SEMAPHORE_H
#define  LOFAR_LCS_THREAD_SEMAPHORE_H

#include <Thread/Condition.h>
#include <Thread/Mutex.h>

 
namespace LOFAR {


class Semaphore
{
  public:
    Semaphore(unsigned level = 0);

    void up(unsigned count = 1);
    bool down(unsigned count = 1);

    void noMore();
    
  private:
    Mutex     mutex;
    Condition condition;
    unsigned  level;
    bool      itsNoMore;
};


inline Semaphore::Semaphore(unsigned level)
:
  level(level),
  itsNoMore(false)
{
}


inline void Semaphore::up(unsigned count)
{
  ScopedLock lock(mutex);
  level += count;
  condition.broadcast();
}


inline bool Semaphore::down(unsigned count)
{
  ScopedLock lock(mutex);

  while (!itsNoMore && level < count)
    condition.wait(mutex);

  if (level >= count) {
    level -= count;
    return true;
  } else {
    return false;
  }
}


inline void Semaphore::noMore()
{
  ScopedLock lock(mutex);
  itsNoMore = true;
  condition.broadcast();
}

} // namespace LOFAR

#endif
