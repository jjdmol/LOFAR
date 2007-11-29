//#  LockedRange.tcc: a lock that is done on a range of any type (int, float etc)
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

namespace LOFAR
{
  namespace CS1
  {

    template < class T, class S >
    LockedRange < T, S >::LockedRange (const S size, const S min,
					 const S max,
					 const T nullOfType):itsSize (size),
      itsMin (min), itsMax (max), itsNullOfType (nullOfType),
      itsFirstItem (nullOfType), itsIsEmpty (true), itsIsOverwriting (true),
      itsWriteLockTimer ("writeLock"), itsWriteUnlockTimer ("writeUnlock"),
      itsReadLockTimer ("readLock"), itsReadUnlockTimer ("readUnlock"),
      itsWaitingForDataTimer ("waitingForData"),
      itsWaitingForSpaceTimer ("waitingForSpace")
    {
      pthread_mutex_init(&itsMutex, 0);
      pthread_cond_init(&itsDataAvailCond, 0);
      pthread_cond_init(&itsSpaceAvailCond, 0);
      clear ();
    };

    template < class T, class S > LockedRange < T, S >::~LockedRange ()
    {
      pthread_mutex_destroy(&itsMutex);
      pthread_cond_destroy(&itsDataAvailCond);
      pthread_cond_destroy(&itsSpaceAvailCond);
      printTimers(clog);
    };

    template < class T, class S >
    void LockedRange < T, S >::printTimers (ostream & os)
    {
      os << "\nLockedRangeTimers:" << endl;
      itsWriteLockTimer.print (os);
      itsWriteUnlockTimer.print (os);
      itsReadLockTimer.print (os);
      itsReadUnlockTimer.print (os);
      itsWaitingForDataTimer.print (os);
      itsWaitingForSpaceTimer.print (os);
    };

    template < class T, class S >
    void LockedRange < T, S >::printPointers ()
    {
      cout << "LockedRange : " << endl
	   << "  itsReadTail  = " << itsReadTail << endl
	   << "  itsReadHead  = " << itsReadHead << endl
	   << "  itsWriteTail = " <<itsWriteTail << endl
	   << "  itsWriteHead = " << itsWriteHead <<endl;
    };

    template < class T, class S >
    T LockedRange < T, S >::writeLock (const T & desiredBegin,
					 const T & desiredEnd)
    {
      itsWriteLockTimer.start ();
      T begin = desiredBegin;
      T end = desiredEnd;
      DBGASSERTSTR (end >= begin,
		    "in LockedRange::writeLock(): end(" << end <<
		    ") should be larger than begin(" << begin << ")");

      pthread_mutex_lock(&itsMutex);

      // check if writeLock was called before
      if (itsIsEmpty)
	{
	  itsFirstItem = begin;
	  itsIsEmpty = false;
	}
      if (begin < itsReadHead)
	{
	  begin = itsReadHead;
	}
      if (itsReadTail == itsNullOfType)
	{
	  itsReadTail = begin;
	  itsReadHead = begin;
	}

      bool amWaiting = false;
      if (end < begin)
	end = begin;
      while ((end - itsReadTail > itsMax) && !itsIsOverwriting)
	{			// && !(itsReadTail==itsNullOfType)) {
	  if (!amWaiting)
	    {
	      itsWriteLockTimer.stop ();
	      cout<<"Waiting for space: "<<begin<<" - "<<end<< endl;;
	      //printPointers();
	      itsWaitingForSpaceTimer.start ();
	      amWaiting = true;
	    }
	  pthread_cond_wait(&itsSpaceAvailCond, &itsMutex);
	}
      if (amWaiting)
	{
	  itsWaitingForSpaceTimer.stop ();
	  cout<<"Space available"<<endl;
	  itsWriteLockTimer.start ();
	}

      if (end > itsWriteHead)
	itsWriteHead = end;
      itsWriteTail = begin;

      if (itsIsOverwriting)
	{
	  T maxReadTail = itsWriteHead - itsMax;
	  if (maxReadTail > itsReadTail)
	    itsReadTail = maxReadTail;
	  if (itsReadHead < itsReadTail)
	    itsReadHead = itsReadTail;
	}

      pthread_mutex_unlock(&itsMutex);
      itsWriteLockTimer.stop ();
      return begin;
    }

    template < class T, class S >
      void LockedRange < T, S >::writeUnlock (const T & end)
    {
      itsWriteUnlockTimer.start ();

      pthread_mutex_lock(&itsMutex);
      itsWriteTail = itsWriteHead = end;
      pthread_cond_broadcast(&itsDataAvailCond);
      pthread_mutex_unlock(&itsMutex);

      itsWriteUnlockTimer.stop ();
    };

    template < class T, class S >
      T LockedRange < T, S >::readLock (const T & desiredBegin,
					const T & desiredEnd)
    {
      itsReadLockTimer.start ();
      T begin = desiredBegin;
      T end = desiredEnd;
      DBGASSERTSTR (end >= begin,
		    "in LockedRange::readLock(): end(" << end <<
		    ") should be larger than begin(" << begin << ")");
      pthread_mutex_lock(&itsMutex);

      itsIsOverwriting = false;

      // if desiredBegin is no longer in the buffer, calculate new begin
      if (itsWriteHead - begin > itsSize)
	{
	  begin = itsWriteHead - itsSize;
	  itsReadTail = begin;
	  pthread_cond_broadcast(&itsSpaceAvailCond);
	}

      if (end < begin)
	end = begin;

      if (begin > itsReadTail) {
        itsReadTail = begin;
        pthread_cond_broadcast(&itsSpaceAvailCond);
      }

      bool amWaiting = false;	
      while (itsWriteTail - end < itsMin)
	{
	  if (!amWaiting)
	    {
	      itsReadLockTimer.stop ();
	      //cout<<"Waiting for data: "<<begin<<" - "<<end<< endl;
      	      //printPointers();
	      itsWaitingForDataTimer.start ();
	      amWaiting = true;
	    }
	  pthread_cond_wait(&itsDataAvailCond, &itsMutex);
	}
      if (amWaiting)
	{
	  itsWaitingForDataTimer.stop ();
	  //cout<<"Data available"<<endl;
	  itsReadLockTimer.start ();
	}
      itsReadHead = end;
      itsReadTail = begin;
      pthread_mutex_unlock(&itsMutex);

      itsReadLockTimer.stop ();
      return begin;
    }

    template < class T, class S >
      void LockedRange < T, S >::readUnlock (const T & end)
    {
      itsReadUnlockTimer.start ();
      pthread_mutex_lock(&itsMutex);
      itsReadTail = itsReadHead = end;
      pthread_cond_broadcast(&itsSpaceAvailCond);
      pthread_mutex_unlock(&itsMutex);
      itsReadUnlockTimer.stop ();
    };

    template < class T, class S > T LockedRange < T, S >::getReadStart ()
    {
      pthread_mutex_lock(&itsMutex);

      bool amWaiting = false;
      while (itsIsEmpty)
	{
	  if (!amWaiting)
	    {
	      cout<<"Waiting for data (empty)"<<endl;
	      itsWaitingForDataTimer.start ();
	      amWaiting = true;
	    }
	  pthread_cond_wait(&itsDataAvailCond, &itsMutex);
	}
      if (amWaiting)
	{
	  itsWaitingForDataTimer.stop ();
	  cout<<"Data available"<<endl;
	}

      T begin = itsFirstItem;
      itsIsOverwriting = false;

      if (itsWriteHead - begin > itsSize)
	{
	  begin = itsWriteHead - itsSize;
	  itsReadTail = begin;
	  pthread_cond_broadcast(&itsSpaceAvailCond);
	}

      itsReadHead = begin;
      itsReadTail = begin;
      pthread_mutex_unlock(&itsMutex);

      return begin;
    }

    template < class T, class S > void LockedRange < T, S >::clear ()
    {
      pthread_mutex_lock(&itsMutex);
      itsReadHead = itsReadTail = itsWriteTail = itsWriteHead = itsNullOfType;
      itsIsOverwriting = true;
      pthread_cond_broadcast(&itsSpaceAvailCond);
      pthread_mutex_unlock(&itsMutex);

      itsIsEmpty = true;
    };

  }				// namespace CS1
}				// namespace LOFAR
