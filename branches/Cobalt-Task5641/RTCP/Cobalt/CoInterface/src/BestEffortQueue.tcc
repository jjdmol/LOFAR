//# BestEffortQueue.tcc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

namespace LOFAR
{
  namespace Cobalt 
  {

template <typename T> inline BestEffortQueue<T>::BestEffortQueue(const std::string &name, size_t maxSize, bool drop)
:
  Queue<T>(name),
  maxSize(maxSize),
  drop(drop),
  dropped("%"),
  flushing(false)
{
}


template <typename T> inline BestEffortQueue<T>::~BestEffortQueue()
{
  LOG_INFO_STR("BestEffortQueue " << Queue<T>::itsName << ": maxSize = " << maxSize << ", dropped = " << dropped.mean() << "%");
}


template <typename T> inline bool BestEffortQueue<T>::_overflow() const
{
  return this->_size > maxSize;
}


template <typename T> inline bool BestEffortQueue<T>::append(T& element, bool timed)
{
  /*
   * Note that if the queue overflows, we drop the FRONT of the queue, that is,
   * the oldest item. That's because the oldest item is less likely to be relevant
   * anymore in the real-time system.
   */

  ScopedLock sl(this->itsMutex);

  if (flushing) {
    dropped.push(100.0);
    return false;
  }

  _append(element, timed);

  if (drop && _overflow()) {
    // drop the head of the queue:
    // 1. bypass the statistics kept by Queue<T>
    // 2. retrieve its value and assign it to `element' to prevent it from being deallocated
    typename Queue<T>::Element e = this->itsQueue.front();
    this->itsQueue.pop_front();
    this->_size--;
    element = e.value;

    dropped.push(100.0);
    return false;
  } else {
    dropped.push(0.0);
    return true;
  }
}


template <typename T> inline void BestEffortQueue<T>::noMore()
{
  if (flushing)
    return;

  // mark queue as flushing
  flushing = true;

  // signal end-of-stream to reader
  Queue<T>::append(0, false);
}


  }
}

