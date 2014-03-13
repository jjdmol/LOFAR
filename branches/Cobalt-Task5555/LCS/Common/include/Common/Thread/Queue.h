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

#ifndef LOFAR_LCS_COMMON_QUEUE_H
#define LOFAR_LCS_COMMON_QUEUE_H

#ifdef USE_THREADS

#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>

#include <list>
#include <time.h>


namespace LOFAR {

template <typename T> class Queue
{
  public:
    Queue() {}

    // Add an element to the back of the queue
    void     append(T);

    // Put an element back to the front of the queue
    void     prepend(T);

    // Remove the front element; waits for an element to be appended
    T	     remove();

    // Remove the front element; waits until `deadline' for an element,
    // and returns `null' if the deadline passed.
    T	     remove(const timespec &deadline, T null);

    unsigned size() const;
    bool     empty() const;

  private:
    Queue(const Queue&);
    Queue& operator=(const Queue&);

    mutable Mutex  itsMutex;
    Condition	   itsNewElementAppended;
    std::list<T>   itsQueue;
};


template <typename T> inline void Queue<T>::append(T element)
{
  ScopedLock scopedLock(itsMutex);

  itsQueue.push_back(element);
  itsNewElementAppended.signal();
}


template <typename T> inline void Queue<T>::prepend(T element)
{
  ScopedLock scopedLock(itsMutex);

  itsQueue.push_front(element);
  itsNewElementAppended.signal();
}


template <typename T> inline T Queue<T>::remove()
{
  ScopedLock scopedLock(itsMutex);

  while (itsQueue.empty())
    itsNewElementAppended.wait(itsMutex);

  T element = itsQueue.front();
  itsQueue.pop_front();

  return element;
}


template <typename T> inline T Queue<T>::remove(const timespec &deadline, T null)
{
#if _POSIX_C_SOURCE >= 199309L
  // Return null if deadline passed
  struct timespec now;
#ifdef CLOCK_REALTIME_COARSE
  clock_gettime(CLOCK_REALTIME_COARSE, &now);
#else
  clock_gettime(CLOCK_REALTIME, &now);
#endif

  if (now.tv_sec > deadline.tv_sec
   || (now.tv_sec == deadline.tv_sec && now.tv_nsec > deadline.tv_nsec))
    return null;
#endif

  ScopedLock scopedLock(itsMutex);

  while (itsQueue.empty())
    if (!itsNewElementAppended.wait(itsMutex, deadline))
      return null;

  T element = itsQueue.front();
  itsQueue.pop_front();

  return element;
}


template <typename T> inline unsigned Queue<T>::size() const
{
  ScopedLock scopedLock(itsMutex);

  return itsQueue.size();
}


template <typename T> inline bool Queue<T>::empty() const
{
  return size() == 0;
}

} // namespace LOFAR

#endif

#endif 
