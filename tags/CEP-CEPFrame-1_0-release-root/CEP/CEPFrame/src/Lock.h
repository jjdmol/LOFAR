//# Lock.h: Pthread implementation of thread locks.
//#
//# Copyright (C) 2000, 2001
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
//# $Id$

#ifndef _LOCK_H_
#define _LOCK_H_

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <pthread.h>

namespace LOFAR
{

#define THREAD_SAFE_MTX  ThreadMutex __pthread_mutex
#define THREAD_SAFE_LOCK ThreadLock  __pthread_lock(__pthread_mutex)

class ThreadMutex
{
 public:
  ThreadMutex()
    {
      pthread_mutex_init(&mutex, NULL);
    }
  
  virtual ~ThreadMutex()
    {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_destroy(&mutex);
    }

  pthread_mutex_t& GetMutex(void)
    {
      return mutex;
    }
  
  void lock(void);
  void unlock(void);
  
 private:
  pthread_mutex_t mutex;
};

inline void ThreadMutex::lock(void)
{
  pthread_mutex_lock(&mutex);
}

inline void ThreadMutex::unlock(void)
{
  pthread_mutex_unlock(&mutex);
}

class ThreadLock
{
 public:
  ThreadLock(ThreadMutex& inMutex) : mutex(inMutex)
    {
      mutex.lock();
    }
  
  virtual ~ThreadLock()
    {
      mutex.unlock();
    }
  
 private:
  ThreadMutex& mutex;
};

class ThreadRWLock
{
 public:
    ThreadRWLock() :
      readers_reading(0), writer_writing(0), max_readers(0)
	{
	    pthread_mutex_init(&mutex, NULL);
	    pthread_cond_init(&lock_free, NULL);
	}

    virtual ~ThreadRWLock()
	{
	}

    // increment the semaphore
    void ReadLock()
	{
	    pthread_mutex_lock(&mutex); //** LOCK

	    // wait for count to become >= 0
	    while (writer_writing)
	    {
		pthread_cond_wait(&lock_free, &mutex);
	    }

	    // increment count, keep max_readers
	    readers_reading++;
	    if (readers_reading > max_readers) max_readers = readers_reading;

	    pthread_mutex_unlock(&mutex); //** UNLOCK
	}

    // decrement the semaphore
    void ReadUnlock()
	{
	    pthread_mutex_lock(&mutex); //** LOCK

	    // something wrong if there are no read locks
	    ASSERTSTR(readers_reading > 0, "Unbalanced ReadUnlock");

	    // one reader less
	    readers_reading--;
	    
	    // tell waiting writer if no readers left
	    if (0 == readers_reading)
	    {
	      pthread_cond_broadcast(&lock_free);
	    }

	    pthread_mutex_unlock(&mutex); //** UNLOCK
	}

    // lock the semaphore (wait until it become 0 and make it -1)
    void WriteLock()
	{
	    pthread_mutex_lock(&mutex); //** LOCK

	    // wait until no one is using the buffer
	    while (writer_writing || readers_reading)
	    {
		pthread_cond_wait(&lock_free, &mutex);
	    }

	    ASSERTSTR(0 == writer_writing, "Multiple writers should not be possible");

	    // one more writer (there should only every be one!)
	    writer_writing = 1;;

	    pthread_mutex_unlock(&mutex); //** UNLOCK
	}

    // unlock the semaphore (make it 0)
    void WriteUnlock()
	{
	    pthread_mutex_lock(&mutex); //** LOCK

	    ASSERTSTR(1 == writer_writing, "Unbalanced WriteUnlock");

	    // no writers left
	    writer_writing = 0;

	    // tell all waiting threads
	    pthread_cond_broadcast(&lock_free);

	    pthread_mutex_unlock(&mutex); //** UNLOCK
	}
    
    int GetReadersReading() { return readers_reading; }
    int GetWriterWriting()  { return writer_writing;  }
    int GetMaxReaders()     { return max_readers;     }

 private:
    pthread_mutex_t mutex;
    pthread_cond_t  lock_free;
    int             readers_reading;
    int             writer_writing;
    int             max_readers;
};

}

#endif
