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

    // The number of elements in the queue. We maintain this info
    // because itsQueue::size() is O(N), at least until C++11.
    size_t itsSize;

    // The time an element spent in a queue
    RunningStatistics retention_time;

    // The percentage of remove() calls on an empty queue
    RunningStatistics remove_on_empty_queue;

    // The average waiting time on remove() if the queue was empty
    RunningStatistics remove_wait_time;

    // The average queue size on append() (excluding the inserted element)
    RunningStatistics queue_size_on_append;

    struct Element {
      T value;
      struct timespec arrival_time;
    };

    mutable Mutex              itsMutex;
    Condition	                 itsNewElementAppended;
    std::list<struct Element>  itsQueue;

    // append() without grabbing itsMutex
    void     unlocked_append(const T&, bool timed);
};


template <typename T> Queue<T>::Queue(const std::string &name)
:
  itsName(name),
  itsSize(0),
  retention_time("s"),
  remove_on_empty_queue("%"),
  remove_wait_time("s"),
  queue_size_on_append("elements")
{
}


template <typename T> Queue<T>::~Queue()
{
  /*
   * Log statistics.
   *
   * Explanation and expected/ideal values:
   *
   * avg #elements on append:  Average size of the queue at append().
   *                      Q holding free items:           large
   *                      Q holding items for processing: 0-1
   *
   * queue empty on remove:    Percentage of calls to remove() that block
   *                      Q holding free items:           0%
   *                      Q holding items for processing: 100%
   *
   * remove wait time:        If queue wasn't empty on remove(), this is the average time before an item was appended
   *                      Q holding free items:           0
   *                      Q holding items for processing: >0
   *
   * element retention time:  The time an element spends between append() and remove()
   *                      Q holding free items:           large
   *                      Q holding items for processing: 0
   *
   */
  if (itsName != "")
    LOG_INFO_STR("Queue " << itsName << ": avg #elements on append = " << queue_size_on_append.mean() << ", queue empty on remove = " << remove_on_empty_queue.mean() << "%, remove wait time = " << remove_wait_time.mean() << " ms, element retention time: " << retention_time);
}


template <typename T> inline void Queue<T>::append(const T& element, bool timed)
{
  ScopedLock scopedLock(itsMutex);

  unlocked_append(element, timed);
}


template <typename T> inline void Queue<T>::unlocked_append(const T& element, bool timed)
{
  Element e;

  // Copy the value to queue
  e.value        = element;

  // Note the time this element entered the queue
  e.arrival_time = timed ? TimeSpec::now() : TimeSpec::big_bang;

  // Record the queue size
  queue_size_on_append.push(itsQueue.size());

  itsQueue.push_back(e);
  itsSize++;

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
  itsSize++;

  itsNewElementAppended.signal();
}


template <typename T> inline T Queue<T>::remove()
{
  using namespace LOFAR::Cobalt::TimeSpec;

  ScopedLock scopedLock(itsMutex);

  const bool beganEmpty = itsQueue.empty();
  const struct timespec begin = TimeSpec::now();

  while (itsQueue.empty())
    itsNewElementAppended.wait(itsMutex);

  Element e = itsQueue.front();
  itsQueue.pop_front();
  itsSize--;

  const struct timespec end = TimeSpec::now();

  // Record waiting time if queue was not empty
  if (beganEmpty) {
    remove_wait_time.push(end - begin);
  }

  // Record whether we'll need to wait
  remove_on_empty_queue.push(beganEmpty ? 100.0 : 0.0);

  // Record the time this element spent in this queue
  if (e.arrival_time != TimeSpec::big_bang)
    retention_time.push(end - e.arrival_time);

  return e.value;
}


template <typename T> inline T Queue<T>::remove(const struct timespec &deadline, T null)
{
  using namespace LOFAR::Cobalt::TimeSpec;

  // Return null if deadline passed
  if (TimeSpec::now() > deadline)
    return null;

  ScopedLock scopedLock(itsMutex);

  const bool beganEmpty = itsQueue.empty();
  const struct timespec begin = TimeSpec::now();

  while (itsQueue.empty())
    if (!itsNewElementAppended.wait(itsMutex, deadline))
      return null;

  Element e = itsQueue.front();
  itsQueue.pop_front();
  itsSize--;

  const struct timespec end = TimeSpec::now();

  // Record waiting time if queue was not empty
  if (beganEmpty) {
    remove_wait_time.push(end - begin);
  }

  // Record whether we'll need to wait
  remove_on_empty_queue.push(beganEmpty ? 1.0 : 0.0);

  // Record the time this element spent in this queue
  if (e.arrival_time != TimeSpec::big_bang)
    retention_time.push(end - e.arrival_time);

  return e.value;
}


template <typename T> inline unsigned Queue<T>::size() const
{
  ScopedLock scopedLock(itsMutex);

  // Note: list::size() is O(N)
  return itsSize;
}


template <typename T> inline bool Queue<T>::empty() const
{
  return size() == 0;
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
