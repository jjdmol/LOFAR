//# CyclicOWBuffer.h: Cyclic buffer interface
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

#ifndef _CYCLIC_OW_BUFFER_H_
#define _CYCLIC_OW_BUFFER_H_

#include <lofar_config.h>

#include "CEPFrame/Lock.h"
#include <Common/lofar_vector.h>

#include "CEPFrame/CyclicBuffer.h"

namespace LOFAR
{

/** A cyclic buffer template, like a fifo. When the buffer is full, the
    oldest values are overwritten. (Cyclic Over Writing Buffer)*/
template <class TYPE>
class CyclicOWBuffer
{
 public:

  CyclicOWBuffer();
  ~CyclicOWBuffer();
  
  // return new ID
  int   AddBufferElement(TYPE DataItem);
  void  RemoveBufferElement(int ID);
  void  RemoveElements();

  TYPE& GetWriteLockedDataItem(int* ID); // Write new item to buffer
  const TYPE& GetReadDataItem(int* ID);  // Read and remove item from buffer
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
  int itsTailIdx;
  int itsCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
};

template<class TYPE>
CyclicOWBuffer<TYPE>::CyclicOWBuffer() :
    itsHeadIdx(0),
    itsTailIdx(0),
    itsCount(0)
{
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_cond_init (&data_available,  NULL);
}

template<class TYPE>
CyclicOWBuffer<TYPE>::~CyclicOWBuffer()
{
  itsBuffer.clear();
}

template<class TYPE>
int CyclicOWBuffer<TYPE>::AddBufferElement(TYPE DataItem)
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
void CyclicOWBuffer<TYPE>::RemoveBufferElement(int ID)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // erase the element at the specified position
  itsBuffer.erase(ID);

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicOWBuffer<TYPE>::RemoveElements(void)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // clear the deque
  itsBuffer.clear();

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
TYPE& CyclicOWBuffer<TYPE>::GetWriteLockedDataItem(int* ID)
{
  pthread_mutex_lock(&buffer_mutex);

  if (itsCount >= (int)itsBuffer.size())  // overwrite oldest value
  {
    AssertStr(itsCount == (int)itsBuffer.size(),
	      "itsCount=" << itsCount << " out of range (" << itsBuffer.size() << ")");

    int ID;
    ID = itsTailIdx++;
    itsBuffer[ID].itsRWLock.ReadLock();
  
    // adjust the tail
    if (itsTailIdx >= (int)itsBuffer.size())
    {
      itsTailIdx = 0;
    }
    itsCount--;

    itsBuffer[ID].itsRWLock.ReadUnlock();
  }
  
  // CONDITION: itsCount < itsBuffer.size()
  // There is now room for at least one element

  AssertStr(itsCount < (int)itsBuffer.size(), "itsCount=" << itsCount 
	    << " out of range (" << itsBuffer.size() << ")");
  
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
const TYPE& CyclicOWBuffer<TYPE>::GetReadDataItem(int* ID)
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

  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[*ID].itsItem;
}

template<class TYPE>
void CyclicOWBuffer<TYPE>::WriteUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteUnlock();
}

template<class TYPE>
void CyclicOWBuffer<TYPE>::ReadUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadUnlock(); 
}

template<class TYPE>
void CyclicOWBuffer<TYPE>::ReadLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadLock();
}

template<class TYPE>
void CyclicOWBuffer<TYPE>::WriteLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteLock();
}

template<class TYPE>
bool CyclicOWBuffer<TYPE>::CheckIDUniqueness(void)
{
  //implicitly true for the deque implementation
  return true;
}

template<class TYPE>
bool CyclicOWBuffer<TYPE>::CheckConsistency(int max)
{
  bool result = false;
  int  i;

  // check internal consistency of CyclicOWBuffer
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
void CyclicOWBuffer<TYPE>::DumpState(void)
{
  pthread_mutex_lock(&buffer_mutex);

  CheckConsistency(1000);
  
  cerr << "itsHeadIdx = " << itsHeadIdx << endl;
  cerr << "itsTailIdx = " << itsTailIdx << endl;
  cerr << "itsCount   = " << itsCount << endl;

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
int CyclicOWBuffer<TYPE>::GetSize(void)
{
  return itsBuffer.size();
}

template<class TYPE>
int CyclicOWBuffer<TYPE>::GetCount(void)
{
  return itsCount;
}

}

#endif
