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
//# $Id: SlidingPointer.h 17975 2011-05-10 09:52:51Z mol $

#ifndef LOFAR_GPUPROC_SLIDING_POINTER_H
#define LOFAR_GPUPROC_SLIDING_POINTER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>

#include <set>


namespace LOFAR {
namespace RTCP {


template <typename T> class SlidingPointer
{
  public:
	 SlidingPointer(T = 0);

    void advanceTo(T);
    void waitFor(T);

  private:
    struct WaitCondition {
      WaitCondition(T value, std::set<WaitCondition *> &set) : value(value), set(set) { set.insert(this); }
      ~WaitCondition() { set.erase(this); }

      T	        value;
      Condition valueReached;
      std::set<WaitCondition *> &set;
    };

    T			      itsValue;
    Mutex		      itsMutex;
    std::set<WaitCondition *> itsWaitList;
};


template <typename T> inline SlidingPointer<T>::SlidingPointer(T value)
:
  itsValue(value)
{
}


template <typename T> inline void SlidingPointer<T>::advanceTo(T value)
{
  ScopedLock lock(itsMutex);

  if (value > itsValue) {
    itsValue = value;

    for (typename std::set<WaitCondition *>::iterator it = itsWaitList.begin(); it != itsWaitList.end(); it ++)
      if (value >= (*it)->value)
	(*it)->valueReached.signal();
  }
}


template <typename T> inline void SlidingPointer<T>::waitFor(T value)
{
  ScopedLock lock(itsMutex);

  while (itsValue < value) {
    WaitCondition waitCondition(value, itsWaitList);
    waitCondition.valueReached.wait(itsMutex);
  }
}

} // namespace RTCP
} // namespace LOFAR

#endif
