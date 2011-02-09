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

#ifndef LOFAR_LCS_THREAD_MUTEX_H
#define LOFAR_LCS_THREAD_MUTEX_H

#include <pthread.h>

#include <Common/SystemCallException.h>


namespace LOFAR {

class Mutex
{
  public:
    Mutex(), ~Mutex();

    void lock(), unlock();
    bool trylock();

  private:
    friend class Condition;
    pthread_mutex_t mutex;
};


class ScopedLock
{
  public:
    ScopedLock(Mutex &);
    ~ScopedLock();

  private:
    Mutex &itsMutex;
};


inline Mutex::Mutex()
{
  int error = pthread_mutex_init(&mutex, 0);

  if (error != 0)
    throw SystemCallException("pthread_mutex_init", error, THROW_ARGS);
}


inline Mutex::~Mutex()
{
  int error = pthread_mutex_destroy(&mutex);

  if (error != 0)
    throw SystemCallException("pthread_mutex_destroy", error, THROW_ARGS);
}


inline void Mutex::lock()
{
  int error = pthread_mutex_lock(&mutex);

  if (error != 0)
    throw SystemCallException("pthread_mutex_destroy", error, THROW_ARGS);
}


inline void Mutex::unlock()
{
  int error = pthread_mutex_unlock(&mutex);

  if (error != 0)
    throw SystemCallException("pthread_mutex_destroy", error, THROW_ARGS);
}


inline bool Mutex::trylock()
{
  int error = pthread_mutex_trylock(&mutex);

  switch (error) {
    case 0     : return true;

    case EBUSY : return false;

    default    : throw SystemCallException("pthread_mutex_trylock", error, THROW_ARGS);
  }
}


inline ScopedLock::ScopedLock(Mutex &mutex)
:
  itsMutex(mutex)
{
  itsMutex.lock();
}


inline ScopedLock::~ScopedLock()
{
  itsMutex.unlock();
}


} // namespace LOFAR

#endif

