//# CyclicBuffer.h: Cyclic buffer interface
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

#ifndef _CYCLIC_BUFFER_H_
#define _CYCLIC_BUFFER_H_

#include <lofar_config.h>

#include <CEPFrame/Lock.h>
//#include <Common/lofar_deque.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

#define CEPF_MIN(a,b) ((a)<(b)?(a):(b))

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

  TYPE& GetWriteLockedDataItem(int* ID); // Write new item to buffer
  const TYPE& GetReadDataItem(int* ID);  // Read and remove item from buffer
  TYPE& GetRWLockedDataItem(int* ID);    // Read and write item in buffer
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
  
  vector< BufferElement <TYPE> > itsBuffer;

  int itsHeadIdx;
  int itsBodyIdx;
  int itsTailIdx;
  int itsCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
  pthread_cond_t  space_available;
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer() :
    itsHeadIdx(0),
    itsBodyIdx(0),
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
TYPE& CyclicBuffer<TYPE>::GetWriteLockedDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until space becomes available
  while (itsCount >= (int)itsBuffer.size())
  {
    ASSERTSTR(itsCount == (int)itsBuffer.size(),
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
TYPE& CyclicBuffer<TYPE>::GetRWLockedDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until data becomes available
  while (itsCount <= 0)
  {
    ASSERTSTR(0 == itsCount, "itsCount=" << itsCount << " out of range (min=0)");
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // CONDITION: itsCount > 0
  // There is at least one element available
  
  *ID = itsBodyIdx++;
  itsBuffer[*ID].itsRWLock.WriteLock();
  
  // adjust the body id
  if (itsBodyIdx >= (int)itsBuffer.size())
  {
    itsBodyIdx = 0;
  }

  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[*ID].itsItem;
}

template<class TYPE>
const TYPE& CyclicBuffer<TYPE>::GetReadDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until data becomes available
  while (itsCount <= 0)
  {
    ASSERTSTR(0 == itsCount, "itsCount=" << itsCount << " out of range (min=0)");
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

  for (i=0; i< CEPF_MIN(max, (int)itsBuffer.size()); i++)
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

}

#endif
