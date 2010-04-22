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

#include <Interface/Mutex.h>

#include <list>


namespace LOFAR {
namespace RTCP {

template <typename T> class Queue
{
  public:
    void     append(T);
    T	     remove();

    unsigned size() const;
    bool     empty() const;

  private:
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


template <typename T> inline T Queue<T>::remove()
{
  ScopedLock scopedLock(itsMutex);

  while (itsQueue.empty())
    itsNewElementAppended.wait(itsMutex);

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

} // namespace RTCP
} // namespace LOFAR

#endif 
