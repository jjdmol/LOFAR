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

#ifndef LOFAR_COINTERFACE_QUEUE_H
#define LOFAR_COINTERFACE_QUEUE_H

#ifdef USE_THREADS

#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/TimeFuncs.h>
#include <CoInterface/RunningStatistics.h>

#include <list>
#include <time.h>
#include <string>


namespace LOFAR {

  namespace Cobalt {

template <typename T> class Queue
{
  public:
    // Create a named queue
    Queue(const std::string &name);

    // Log queue statistics
    ~Queue();

    // Add an element to the back of the queue.
    //
    // If timed, this element is taken into account for timing statistics.
    //
    // Untimed items include:
    //   * Elements added to initially fill the queue, waiting for obs start.
    //   * Control elements, such as NULL.
    void     append(const T&, bool timed = true);

    // Put an element back to the front of the queue
    void     prepend(const T&);

    // Remove the front element; waits for an element to be appended
    T	       remove();

    // Remove the front element; waits until `deadline' for an element,
    // and returns `null' if the deadline passed.
    T	       remove(const struct timespec &deadline, T null);

    unsigned size() const;
    bool     empty() const;
    struct timespec oldest() const;

  private:
    Queue(const Queue&);
    Queue& operator=(const Queue&);

  protected:
    const std::string itsName;

    // The time an element spent in a queue
    RunningStatistics retention_time;

    // The fraction of remove() calls on an empty queue
    RunningStatistics remove_on_empty_queue;

    // The average queue size on append() (excluding the inserted element)
    RunningStatistics queue_size_on_add;

    struct Element {
      T value;
      struct timespec arrival_time;
    };

    mutable Mutex              itsMutex;
    Condition	                 itsNewElementAppended;
    std::list<struct Element>  itsQueue;
};


template <typename T> Queue<T>::Queue(const std::string &name)
:
  itsName(name)
{
}


template <typename T> Queue<T>::~Queue()
{
  if (itsName != "")
    LOG_INFO_STR("Queue " << itsName << ": avg #elements @add = " << queue_size_on_add.mean() << ", queue empty @remove = " << remove_on_empty_queue.mean() * 100.0 << "%, element retention time: " << retention_time);
}


template <typename T> inline void Queue<T>::append(const T& element, bool timed)
{
  ScopedLock scopedLock(itsMutex);

  Element e;

  // Copy the value to queue
  e.value        = element;

  // Note the time this element entered the queue
  e.arrival_time = timed ? TimeSpec::now() : TimeSpec::big_bang;

  // Record the queue size
  queue_size_on_add.push(itsQueue.size());

  itsQueue.push_back(e);
  itsNewElementAppended.signal();
}


template <typename T> inline void Queue<T>::prepend(const T& element)
{
  ScopedLock scopedLock(itsMutex);

  Element e;

  // Copy the value to queue
  e.value        = element;

  // We don't record an arrival time, since the element is likely
  // pushed back ("unget"). Recording an arrival time here would
  // screw up statistics.
  e.arrival_time = TimeSpec::big_bang;

  itsQueue.push_front(e);
  itsNewElementAppended.signal();
}


template <typename T> inline T Queue<T>::remove()
{
  using namespace LOFAR::Cobalt::TimeSpec;

  ScopedLock scopedLock(itsMutex);

  // Record whether we'll need to wait
  remove_on_empty_queue.push(itsQueue.empty() ? 1.0 : 0.0);

  while (itsQueue.empty())
    itsNewElementAppended.wait(itsMutex);

  Element e = itsQueue.front();
  itsQueue.pop_front();

  // Record the time this element spent in this queue
  if (e.arrival_time != TimeSpec::big_bang)
    retention_time.push(TimeSpec::now() - e.arrival_time);

  return e.value;
}


template <typename T> inline T Queue<T>::remove(const struct timespec &deadline, T null)
{
  using namespace LOFAR::Cobalt::TimeSpec;

  // Return null if deadline passed
  if (TimeSpec::now() > deadline)
    return null;

  ScopedLock scopedLock(itsMutex);

  // Record whether we'll need to wait
  remove_on_empty_queue.push(itsQueue.empty() ? 1.0 : 0.0);

  while (itsQueue.empty())
    if (!itsNewElementAppended.wait(itsMutex, deadline))
      return null;

  Element e = itsQueue.front();
  itsQueue.pop_front();

  // Record the time this element spent in this queue
  if (e.arrival_time != TimeSpec::big_bang)
    retention_time.push(TimeSpec::now() - e.arrival_time);

  return e.value;
}


template <typename T> inline unsigned Queue<T>::size() const
{
  ScopedLock scopedLock(itsMutex);

  // Note: list::size() is O(N)
  return itsQueue.size();
}


template <typename T> inline bool Queue<T>::empty() const
{
  ScopedLock scopedLock(itsMutex);

  // Note: list::empty() is O(1)
  return itsQueue.empty();
}


template <typename T> inline struct timespec Queue<T>::oldest() const
{
  ScopedLock scopedLock(itsMutex);

  return itsQueue.empty() ? TimeSpec::now() : itsQueue.front().arrival_time;
}

} // namespace Cobalt

} // namespace LOFAR

#endif

#endif 
