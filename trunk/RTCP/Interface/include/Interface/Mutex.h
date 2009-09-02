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
//#  $Id$

#ifndef LOFAR_INTERFACE_MUTEX_H
#define LOFAR_INTERFACE_MUTEX_H

#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace LOFAR {
namespace RTCP {

class Mutex: boost::noncopyable
{
  public:
    Mutex(), ~Mutex();

    void lock(), unlock();
    bool trylock();

  private:
    pthread_mutex_t mutex;
};

inline Mutex::Mutex()
{
  pthread_mutex_init(&mutex, 0);
}


inline Mutex::~Mutex()
{
  pthread_mutex_destroy(&mutex);
}


inline void Mutex::lock()
{
  pthread_mutex_lock(&mutex);
}


inline void Mutex::unlock()
{
  pthread_mutex_unlock(&mutex);
}


inline bool Mutex::trylock()
{
  return pthread_mutex_trylock(&mutex) == 0;
}

} // namespace RTCP
} // namespace LOFAR

#endif

