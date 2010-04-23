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

#include <pthread.h>

 
namespace LOFAR {


class Semaphore
{
  public:
    Semaphore(unsigned level = 0);
    ~Semaphore();

    void up(unsigned count = 1);
    bool down(unsigned count = 1);

    void noMore();
    
  private:
    pthread_mutex_t mutex;
    pthread_cond_t  condition;
    unsigned	    level;
    bool	    itsNoMore;
};


inline Semaphore::Semaphore(unsigned level)
:
  level(level),
  itsNoMore(false)
{
  pthread_mutex_init(&mutex, 0);
  pthread_cond_init(&condition, 0);
}


inline Semaphore::~Semaphore()
{
  pthread_cond_destroy(&condition);
  pthread_mutex_destroy(&mutex);
}


inline void Semaphore::up(unsigned count)
{
  pthread_mutex_lock(&mutex);
  level += count;
  pthread_cond_broadcast(&condition); // pthread_cond_signal() is incorrect
  pthread_mutex_unlock(&mutex);
}


inline bool Semaphore::down(unsigned count)
{
  pthread_mutex_lock(&mutex);

  while (!itsNoMore && level < count)
    pthread_cond_wait(&condition, &mutex);

  if (level >= count) {
    level -= count;
    pthread_mutex_unlock(&mutex);
    return true;
  } else {
    pthread_mutex_unlock(&mutex);
    return false;
  }
}


inline void Semaphore::noMore()
{
  pthread_mutex_lock(&mutex);
  itsNoMore = true;
  pthread_cond_broadcast(&condition);
  pthread_mutex_unlock(&mutex);
}

} // namespace LOFAR

#endif
