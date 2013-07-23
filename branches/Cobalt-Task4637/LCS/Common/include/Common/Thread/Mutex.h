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
    enum Type {
#ifdef USE_THREADS
      NORMAL = PTHREAD_MUTEX_NORMAL,
      RECURSIVE = PTHREAD_MUTEX_RECURSIVE,
      ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK,
      DEFAULT = PTHREAD_MUTEX_DEFAULT
#else
      NORMAL,
      RECURSIVE,
      ERRORCHECK,
      DEFAULT
#endif
    };
    
    Mutex(Type type=DEFAULT);
    ~Mutex();

    void lock();
    void unlock();
    bool trylock();

  private:
    friend class Condition;

    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
    
#ifdef USE_THREADS    
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexattr;
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


inline Mutex::Mutex(Mutex::Type type)
{
#ifdef USE_THREADS
  int error;

  error = pthread_mutexattr_init(&mutexattr);
  if (error != 0)
    throw SystemCallException("pthread_mutexattr_init", error, THROW_ARGS);
    
  error = pthread_mutexattr_settype(&mutexattr, type);
  if (error != 0)
    throw SystemCallException("pthread_mutexattr_settype", error, THROW_ARGS);

  error = pthread_mutex_init(&mutex, &mutexattr);
  if (error != 0)
    throw SystemCallException("pthread_mutex_init", error, THROW_ARGS);
#else
  (void)type;
#endif    
}


inline Mutex::~Mutex()
{
#ifdef USE_THREADS
  // We can't log any errors because the logger will also use this mutex
  // class. So it's no use recording the return value.
  (void)pthread_mutex_destroy(&mutex);
  (void)pthread_mutexattr_destroy(&mutexattr);
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
    case 0       : return true;

    case EBUSY   : return false;

    // According to the POSIX standard only pthread_mutex_lock() can return
    // EDEADLK. However, some Linux implementations also return EDEADLK when
    // calling pthread_mutex_trylock() on a locked error-checking mutex.
    case EDEADLK : return false;

    default      : throw SystemCallException("pthread_mutex_trylock", error, THROW_ARGS);
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

