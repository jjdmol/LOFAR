//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: Mutex.h 15519 2010-04-22 10:00:35Z romein $

#ifndef LOFAR_LCS_THREAD_CONDITION_H
#define LOFAR_LCS_THREAD_CONDITION_H

#include <pthread.h>

#include <Common/SystemCallException.h>
#include <Thread/Mutex.h>


namespace LOFAR {


class Condition
{
  public:
    Condition(), ~Condition();

    void signal(), broadcast();
    void wait(Mutex &);
    bool wait(Mutex &, const struct timespec &);

  private:
    pthread_cond_t condition;
};


inline Condition::Condition()
{
  int error = pthread_cond_init(&condition, 0);

  if (error != 0)
    throw SystemCallException("pthread_cond_init", error, THROW_ARGS);
}


inline Condition::~Condition()
{
  int error = pthread_cond_destroy(&condition);

  if (error != 0)
    throw SystemCallException("pthread_cond_destroy", error, THROW_ARGS);
}


inline void Condition::signal()
{
  int error = pthread_cond_signal(&condition);

  if (error != 0)
    throw SystemCallException("pthread_cond_signal", error, THROW_ARGS);
}


inline void Condition::broadcast()
{
  int error = pthread_cond_broadcast(&condition);

  if (error != 0)
    throw SystemCallException("pthread_cond_broadcast", error, THROW_ARGS);
}


inline void Condition::wait(Mutex &mutex)
{
  int error = pthread_cond_wait(&condition, &mutex.mutex);

  if (error != 0)
    throw SystemCallException("pthread_cond_wait", error, THROW_ARGS);
}


inline bool Condition::wait(Mutex &mutex, const struct timespec &timespec)
{
  int error = pthread_cond_timedwait(&condition, &mutex.mutex, &timespec);

  switch (error) {
    case 0	   : return true;

    case ETIMEDOUT : return false;

    default	   : throw SystemCallException("pthread_cond_timedwait", error, THROW_ARGS);
  }
}


} // namespace LOFAR

#endif

