//////////////////////////////////////////////////////////////////////
//
//  CyclicBuffer.h: Cyclic buffer interface
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.13  2002/10/15 11:36:48  wierenga
//
//    %[BugId: 102 ]%
//
//  Test checkin. Should work now.
//
//  Revision 1.12  2002/10/15 11:24:27  wierenga
//  %[BugId:102]%
//  Add missing call to pthread_mutex_lock in CyclicBuffer::AddBufferElement.
//  This error was found by valgrind!
//
//  Revision 1.11  2002/07/05 09:13:36  wierenga
//
//  %[BugId: 33]%
//
//  Finished implementation and documentation. See LOFAR-ASTRON-RPT-016.
//
//  Revision 1.10  2002/06/13 11:56:53  wierenga
//  %[BugId: 2]%
//  Remove maxclients argument from CyclicBuffer constructor, it is not needed.
//
//  Revision 1.9  2002/06/04 14:42:10  wierenga
//  %[BugId:28]%
//  First stab at using CyclicBuffer.
//
//  Revision 1.8  2002/06/04 12:25:12  wierenga
//  %[BugId:2]%
//  Fixed bug in CyclicBufferTest. The UNIX signal facility does not
//  combine well with pthreads :-). The signal handler was only used
//  to periodically print the status of the circular buffer. After taking
//  out the SIGALRM signal the code works fine.
//
//  Revision 1.7  2002/06/04 07:23:47  wierenga
//  %[BugId:2]%
//  Use pthread_cond_broadcast instead of pthread_cond_signal.
//
//  Revision 1.6  2002/05/28 14:33:52  wierenga
//  %[BugId:2]%
//  Trying to get the CyclicBufferTest to work on multi-CPU machine (lofar8).
//  It is currently failing in some cases.
//
//  Revision 1.5  2002/05/28 08:35:15  wierenga
//  %[BugId: 2]%
//  Signal space_available and data_available before unlocking the buffer.
//
//  Revision 1.4  2002/05/28 07:44:52  wierenga
//  %[BugId:2]%
//  Needed global lock around global counter.
//  Moved Lock.h from BaseSim/Corba to BaseSim.
//
//  Revision 1.3  2002/05/24 14:39:49  wierenga
//  %[BugId: 2]%
//  First completed version of the CyclicBuffer implementation.
//  This includes a test program in BaseSim/test which will be run
//  if you run 'make check'.
//
//  Revision 1.2  2002/05/23 11:18:47  wierenga
//  %[BugId: 2]%
//  Initial version of CyclicBuffer which compiles correctly.
//
//  Revision 1.1  2002/05/23 07:23:50  wierenga
//  %[BugId: 2]%
//
//  New files for CyclicBuffer class.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef _CYCLIC_BUFFER_H_
#define _CYCLIC_BUFFER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Lock.h"
#include <Common/lofar_deque.h>

#define MIN(a,b) ((a)<(b)?(a):(b))

// an element of the CyclicBuffer
template <class TYPE>
class BufferElement
{
 public:
  BufferElement(TYPE DataItem) :
    itsItem(DataItem)
    {
    }

    // Control single write multiple read access
    ThreadRWLock itsRWLock;
  
    // Pointer to the TYPE object.
    TYPE itsItem;

 private:
    
    // size of the TYPE
    int   itsSize;
    
    // Unique ID of this BufferElement.
    int itsID;
};

/** A cyclic buffer template, like a fifo */
template <class TYPE>
class CyclicBuffer
{
 public:

  CyclicBuffer();
  ~CyclicBuffer();
  
  // return new ID
  int   AddBufferElement(TYPE DataItem);
  void  RemoveBufferElement(int ID);
  void  RemoveElements();

  TYPE  GetWriteLockedDataItem(int* ID);
  TYPE  GetReadDataItem(int* ID);
  TYPE  GetDataItem(int ID);
  void  WriteUnlockElement(int ID);
  void  ReadUnlockElement(int ID);

  bool CheckConsistency(int max);
  void DumpState();

  int  GetSize();
  int  GetCount();

 private:

  void ReadLockElement(int ID);
  void WriteLockElement(int ID);
  bool CheckIDUniqueness();
  
 protected:
  deque< BufferElement <TYPE> > itsBuffer;

  int itsHeadIdx;
  int itsTailIdx;
  int itsCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
  pthread_cond_t  space_available;
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer() :
    itsHeadIdx(0),
    itsTailIdx(0),
    itsCount(0)
{
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_cond_init (&data_available,  NULL);
  pthread_cond_init (&space_available, NULL);
}

template<class TYPE>
CyclicBuffer<TYPE>::~CyclicBuffer()
{
  itsBuffer.clear();
}

template<class TYPE>
int CyclicBuffer<TYPE>::AddBufferElement(TYPE DataItem)
{
  BufferElement<TYPE> elem(DataItem);

  pthread_mutex_lock(&buffer_mutex);

  // add the element at the back of the deque
  itsBuffer.push_back(elem);

  pthread_mutex_unlock(&buffer_mutex);

  // the ID will be the index in the deque
  return (itsBuffer.size()-1);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveBufferElement(int ID)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // erase the element at the specified position
  itsBuffer.erase(ID);

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveElements(void)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // clear the deque
  itsBuffer.clear();

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetWriteLockedDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until space becomes available
  while (itsCount >= (int)itsBuffer.size())
  {
    AssertStr(itsCount == (int)itsBuffer.size(),
	      "itsCount=" << itsCount << " out of range (" << itsBuffer.size() << ")");
    pthread_cond_wait(&space_available, &buffer_mutex);
  }
  
  // CONDITION: itsCount < itsBuffer.size()
  // There is now room for at least one element
  
  *ID = itsHeadIdx;
  itsBuffer[*ID].itsRWLock.WriteLock();
  
  // adjust the head (points to first free position)
  itsHeadIdx++;
  if (itsHeadIdx >= (int)itsBuffer.size())
  {
    itsHeadIdx = 0;
  }
  itsCount++;

  // signal that data has become available
  if (1 == itsCount)
  {
    pthread_cond_broadcast(&data_available);
  }

  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[*ID].itsItem;
}

template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetReadDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until data becomes available
  while (itsCount <= 0)
  {
    AssertStr(0 == itsCount, "itsCount=" << itsCount << " out of range (min=0)");
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // CONDITION: itsCount > 0
  // There is at least one element available
  
  *ID = itsTailIdx++;
  itsBuffer[*ID].itsRWLock.ReadLock();
  
  // adjust the tail
  if (itsTailIdx >= (int)itsBuffer.size())
  {
    itsTailIdx = 0;
  }
  itsCount--;

  // signal that space has become available

  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[*ID].itsItem;
}

template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetDataItem(int ID)
{
  return itsBuffer[ID].itsItem;
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteUnlock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadUnlock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadLock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteLock();
}

template<class TYPE>
bool CyclicBuffer<TYPE>::CheckIDUniqueness(void)
{
  //implicitly true for the deque implementation
  return true;
}

template<class TYPE>
bool CyclicBuffer<TYPE>::CheckConsistency(int max)
{
  bool result = false;
  int  i;

  // check internal consistency of CyclicBuffer
  // all locks should be released

  for (i=0; i< MIN(max, (int)itsBuffer.size()); i++)
  {
    cerr << "elem("  << i << "): readers_reading=" << 
      itsBuffer[i].itsRWLock.GetReadersReading() <<
      ", writer_writing=" << itsBuffer[i].itsRWLock.GetWriterWriting() <<
      ", maxcount=" << itsBuffer[i].itsRWLock.GetMaxReaders() << endl;
  }

  return result;
}

template<class TYPE>
void CyclicBuffer<TYPE>::DumpState(void)
{
  pthread_mutex_lock(&buffer_mutex);

  CheckConsistency(1000);
  
  cerr << "itsHeadIdx = " << itsHeadIdx << endl;
  cerr << "itsTailIdx = " << itsTailIdx << endl;
  cerr << "itsCount   = " << itsCount << endl;

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
int CyclicBuffer<TYPE>::GetSize(void)
{
  return itsBuffer.size();
}

template<class TYPE>
int CyclicBuffer<TYPE>::GetCount(void)
{
  return itsCount;
}

#endif
