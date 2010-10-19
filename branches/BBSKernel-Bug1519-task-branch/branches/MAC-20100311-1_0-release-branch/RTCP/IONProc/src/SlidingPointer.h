//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_IONPROC_SLIDING_POINTER_H
#define LOFAR_IONPROC_SLIDING_POINTER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <pthread.h>


namespace LOFAR {
namespace RTCP {


template <typename T> class SlidingPointer
{
  public:
	 SlidingPointer();
	 SlidingPointer(const T &);
	 ~SlidingPointer();

    void advanceTo(const T &);
    void waitFor(const T &);

  private:
    T		    itsValue, itsWaitingForValue;
    pthread_mutex_t itsMutex;
    pthread_cond_t  itsAwaitedValueReached;
    bool	    itsIsWaiting;
};


template <typename T> inline SlidingPointer<T>::SlidingPointer()
:
  itsIsWaiting(false)
{
  pthread_mutex_init(&itsMutex, 0);
  pthread_cond_init(&itsAwaitedValueReached, 0);
}


template <typename T> inline SlidingPointer<T>::SlidingPointer(const T &value)
:
  itsValue(value),
  itsIsWaiting(false)
{
  pthread_mutex_init(&itsMutex, 0);
  pthread_cond_init(&itsAwaitedValueReached, 0);
}


template <typename T> inline SlidingPointer<T>::~SlidingPointer()
{
  pthread_mutex_destroy(&itsMutex);
  pthread_cond_destroy(&itsAwaitedValueReached);
}


template <typename T> inline void SlidingPointer<T>::advanceTo(const T &value)
{
  pthread_mutex_lock(&itsMutex);

  if (value > itsValue) {
    itsValue = value;

    if (itsIsWaiting && value >= itsWaitingForValue)
      pthread_cond_signal(&itsAwaitedValueReached);
  }

  pthread_mutex_unlock(&itsMutex);
}


template <typename T> inline void SlidingPointer<T>::waitFor(const T &value)
{
  pthread_mutex_lock(&itsMutex);

  while (itsValue < value) {
    itsIsWaiting       = true;
    itsWaitingForValue = value;
    pthread_cond_wait(&itsAwaitedValueReached, &itsMutex);
    itsIsWaiting       = false;
  }

  pthread_mutex_unlock(&itsMutex);
}

} // namespace RTCP
} // namespace LOFAR

#endif
