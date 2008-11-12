//# Queue.h:
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_INTERFACE_QUEUE_H
#define LOFAR_INTERFACE_QUEUE_H

#include <pthread.h>
#include <list>


namespace LOFAR {
namespace RTCP {

template <typename T> class Queue
{
  public:
	     Queue();
	     ~Queue();

    void     append(T);
    T	     remove();

    unsigned size() const;
    bool     empty() const;

  private:
    mutable pthread_mutex_t mutex;
    pthread_cond_t	    newElementAppended;
    std::list<T>	    queue;
};


template <typename T> inline Queue<T>::Queue()
{
  pthread_mutex_init(&mutex, 0);
  pthread_cond_init(&newElementAppended, 0);
}


template <typename T> inline Queue<T>::~Queue()
{
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&newElementAppended);
}


template <typename T> inline void Queue<T>::append(T element)
{
  pthread_mutex_lock(&mutex);
  queue.push_back(element);
  pthread_cond_signal(&newElementAppended);
  pthread_mutex_unlock(&mutex);
}


template <typename T> inline T Queue<T>::remove()
{
  pthread_mutex_lock(&mutex);

  while (queue.empty())
    pthread_cond_wait(&newElementAppended, &mutex);

  T element = queue.front();
  queue.pop_front();
  pthread_mutex_unlock(&mutex);

  return element;
}


template <typename T> inline unsigned Queue<T>::size() const
{
  unsigned size;

  pthread_mutex_lock(&mutex);
  size = queue.size();
  pthread_mutex_unlock(&mutex);

  return size;
}


template <typename T> inline bool Queue<T>::empty() const
{
  return size() == 0;
}

} // namespace RTCP
} // namespace LOFAR

#endif 
