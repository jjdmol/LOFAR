//#  LockedRange.h: a lock that is done on a range of any type (int, float etc)
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INPUTSECTION_LOCKEDRANGE_H
#define LOFAR_CS1_INPUTSECTION_LOCKEDRANGE_H

// \file
// a lock that is done on a range of any type (int, float etc)

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <boost/thread.hpp>
#include <Common/Timer.h>
#include <ostream>

namespace LOFAR 
{
  namespace CS1_InputSection 
  {

    using namespace boost;

    // \addtogroup CS1_InputSection
    // @{

    // This class was written in such a way that T can be any type that supports assignment and comparison operators
    // When using this class with integers, beware that the upper value of a range is not included in the range
    // The class S is the result type of a subtraction of two variables of type T
    template<class T, class S>
    class LockedRange
    {
    public:
      LockedRange(const S size, const S min, const S max, const T nullOfType);
      ~LockedRange();

      T writeLock(const T& desiredBegin, const T& desiredEnd);
      void writeUnlock(const T& end);

      T readLock(const T& desiredBegin, const T& desiredEnd);
      void readUnlock(const T& end);

      T getReadStart();

      void clear();

      void setOverwriting (bool o) {itsIsOverwriting = o; };

    private:
      // Copying is not allowed
      LockedRange (const LockedRange& that);
      LockedRange& operator= (const LockedRange& that);

      void printTimers(ostream& os);

      //# Datamembers
      const S itsSize;
      const S itsMin;
      const S itsMax;
      const T itsNullOfType;

      T itsReadHead;
      T itsReadTail;
      T itsWriteHead;
      T itsWriteTail;
      T itsFirstItem;
      bool itsIsEmpty;
  
      mutex itsMutex;
      condition itsDataAvailCond;
      condition itsSpaceAvailCond;

      // If this buffer IsOverwriting the readtail and readhead are shifted when the buffer is full
      // this means old data is lost, but all new data is stored in the buffer
      bool itsIsOverwriting;

      NSTimer itsWriteLockTimer;
      NSTimer itsWriteUnlockTimer;
      NSTimer itsReadLockTimer;
      NSTimer itsReadUnlockTimer;
      NSTimer itsWaitingForDataTimer;
      NSTimer itsWaitingForSpaceTimer;
    };

    // @}

  } // namespace CS1_InputSection
} // namespace LOFAR

#include <CS1_InputSection/LockedRange.tcc>

#endif
