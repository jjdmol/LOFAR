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

#ifndef LOFAR_LCS_COMMON_MUTEX_H
#define LOFAR_LCS_COMMON_MUTEX_H

#ifdef USE_THREADS
#include <pthread.h>
#include <Common/SystemCallException.h>
#endif

#include <exception>

namespace LOFAR {

class Mutex
{
  public:
    Mutex(), ~Mutex();

    void lock(), unlock();
    bool trylock();

  private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

    friend class Condition;
#ifdef USE_THREADS    
    pthread_mutex_t mutex;
#endif    
};


class ScopedLock
{
  public:
    ScopedLock(Mutex &);
    ~ScopedLock();

  private:
    ScopedLock(const ScopedLock&);
    ScopedLock& operator=(const ScopedLock&);

    Mutex &itsMutex;
};


inline Mutex::Mutex()
{
#ifdef USE_THREADS
  int error = pthread_mutex_init(&mutex, 0);

  if (error != 0)
    throw SystemCallException("pthread_mutex_init", error, THROW_ARGS);
#endif    
}


inline Mutex::~Mutex()
{
#ifdef USE_THREADS
  // We can't log any errors because the logger will also use this mutex class.
  // So it's no use recording the return value.
  (void)pthread_mutex_destroy(&mutex);
#endif    
}


inline void Mutex::lock()
{
#ifdef USE_THREADS
  int error = pthread_mutex_lock(&mutex);

  if (error != 0)
    throw SystemCallException("pthread_mutex_lock", error, THROW_ARGS);
#endif    
}


inline void Mutex::unlock()
{
#ifdef USE_THREADS
  int error = pthread_mutex_unlock(&mutex);

  if (error != 0)
    throw SystemCallException("pthread_mutex_unlock", error, THROW_ARGS);
#endif    
}


inline bool Mutex::trylock()
{
#ifdef USE_THREADS
  int error = pthread_mutex_trylock(&mutex);

  switch (error) {
    case 0     : return true;

    case EBUSY : return false;

    default    : throw SystemCallException("pthread_mutex_trylock", error, THROW_ARGS);
  }
#else
  return true;
#endif    
}


inline ScopedLock::ScopedLock(Mutex &mutex)
:
  itsMutex(mutex)
{
  itsMutex.lock();
}


inline ScopedLock::~ScopedLock()
{
  try {
    itsMutex.unlock();
  } catch (std::exception &) {}
}


} // namespace LOFAR

#endif

