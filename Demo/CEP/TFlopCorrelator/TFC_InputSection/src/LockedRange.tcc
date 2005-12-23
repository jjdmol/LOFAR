//#  LockedRange.tcc: one line description
//#
//#  Copyright (C) 2005
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

namespace LOFAR {

  template<class T, class S>
  LockedRange<T, S>::LockedRange(const S size, const S min, const S max, const T nullOfType) :
    itsSize(size), 
    itsMin(min), 
    itsMax(max), 
    itsNullOfType(nullOfType), 
    itsIsOverwriting(true),
    itsWriteLockTimer("writeLock"),
    itsWriteUnlockTimer("writeUnlock"),
    itsReadLockTimer("readLock"),
    itsReadUnlockTimer("readUnlock"),
    itsWaitingForDataTimer("waitingForData"),
    itsWaitingForSpaceTimer("waitingForSpace")
  { 
    clear();
  };

  template<class T, class S>
  LockedRange<T, S>::~LockedRange(){
    printTimers(cout);
  };

  template<class T, class S>
  void LockedRange<T, S>::printTimers(ostream& os){
    os<<"\nLockedRangeTimers:"<<endl;
    itsWriteLockTimer.print(os);
    itsWriteUnlockTimer.print(os);
    itsReadLockTimer.print(os);
    itsReadUnlockTimer.print(os);
    itsWaitingForDataTimer.print(os);
    itsWaitingForSpaceTimer.print(os);	  
  };

  template<class T, class S>
  T LockedRange<T, S>::writeLock(const T& desiredBegin, const T& desiredEnd) {
    itsWriteLockTimer.start();
    T begin = desiredBegin;
    T end = desiredEnd;
    DBGASSERTSTR(end >= begin, "in LockedRange::writeLock(): end("<<end<<") should be larger than begin("<<begin<<")");
    
    mutex::scoped_lock sl(itsMutex);
    
    if (begin < itsReadHead) {
      begin = itsReadHead;
    }
    
    bool amWaiting = false;
    if (end > begin) {
      while ((end - itsReadTail > itsMax) && !itsIsOverwriting && !(itsReadTail==itsNullOfType)) {
	if (!amWaiting) {
	  itsWriteLockTimer.stop();
	  itsWaitingForSpaceTimer.start();		
	  amWaiting = true;
	}
	itsSpaceAvailCond.wait(sl);
      }
      if (amWaiting) {
	itsWaitingForSpaceTimer.stop();
	itsWriteLockTimer.start();
      }
    } else {
      end = begin;
    }
    
    if (end > itsWriteHead) itsWriteHead = end;
    if (begin < itsWriteTail) itsWriteTail = begin;
    
    if (itsIsOverwriting) {
      T maxReadTail = itsWriteHead - itsMax;
      if (maxReadTail > itsReadTail) itsReadTail = maxReadTail;
      if (itsReadHead < itsReadTail) itsReadHead = itsReadTail;
    }
    
    itsWriteLockTimer.stop();
    return begin;
  }

  template<class T, class S>
  void LockedRange<T, S>::writeUnlock(const T& end) {
    itsWriteUnlockTimer.start(); 

    mutex::scoped_lock sl(itsMutex); 
    itsWriteTail = itsWriteHead = end; 
    itsDataAvailCond.notify_all(); 
  
    itsWriteUnlockTimer.stop();
  };

  template<class T, class S>
  T LockedRange<T, S>::readLock(const T& desiredBegin, const T& desiredEnd) {
    itsReadLockTimer.start();
    T begin = desiredBegin;
    T end = desiredEnd;
    DBGASSERTSTR(end >= begin, "in LockedRange::readLock(): end("<<end<<") should be larger than begin("<<begin<<")");
    mutex::scoped_lock sl(itsMutex);

    itsIsOverwriting = false;

    if (itsWriteHead - begin > itsSize) {
      begin = itsWriteHead - itsSize;
    }
    if (end > begin) {
      bool amWaiting = false;
      while (itsWriteTail - end < itsMin) {
	if (!amWaiting) {
	  itsReadLockTimer.stop();
	  itsWaitingForDataTimer.start();
	  amWaiting = true;
	}
	itsDataAvailCond.wait(sl);
      }
      if (amWaiting) {
	itsWaitingForDataTimer.stop();
	itsReadLockTimer.start();
      }
      itsReadHead = end;
      itsReadTail = begin;
    }
    itsReadLockTimer.stop();
    return begin;
  }

  template<class T, class S>
  void LockedRange<T, S>::readUnlock(const T& end) {
    itsReadUnlockTimer.start(); 
    mutex::scoped_lock sl(itsMutex); 
    itsReadTail = itsReadHead = end; 
    itsSpaceAvailCond.notify_all(); 
    itsReadUnlockTimer.stop();
  };

  template<class T, class S>
  void LockedRange<T, S>::clear() {
    mutex::scoped_lock sl(itsMutex); 
    itsReadHead = itsReadTail = itsWriteTail = itsWriteHead; 
    itsIsOverwriting = true; 
    itsSpaceAvailCond.notify_all();
  };

} // namespace LOFAR
