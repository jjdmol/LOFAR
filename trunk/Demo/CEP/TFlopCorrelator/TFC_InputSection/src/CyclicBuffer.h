//# CyclicBuffer.h: Cyclic buffer interface.
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

#ifndef TFLOPCORRELATOR_CYCLICBUFFER_H
#define TFLOPCORRELATOR_CYCLIC_BUFFER_H

#include <CEPFrame/Lock.h>
#include <Common/lofar_vector.h>

// minumum written elements before reading is allowed
#define MIN_COUNT   1 

// maximum of written elements in buffer to avoid that 
// writeptr is catching up readptr
#define MAX_COUNT  ((int)itsBuffer.size()-4)    

namespace LOFAR
{

// Element of the CyclicBuffer
template <class TYPE>
class BufferElement
{
 public:
  BufferElement(TYPE DataElement) :
    itsElement(DataElement)
    {
    }
    
    // Control single write multiple read access
    ThreadRWLock itsRWLock;
    
    // Pointer to the TYPE object.
    TYPE itsElement;

 private:
    
    // size of the TYPE
    int   itsSize;
    
    // Unique ID of this BufferElement.
    int itsID;
};


template <class TYPE>
class CyclicBuffer
{
 public:

  CyclicBuffer(int nelements);
  ~CyclicBuffer();
  

  // write an item and adjust head and count
  TYPE* getAutoWritePtr(int& ID);

  // read oldest item and adjust tail and count
  TYPE* getAutoReadPtr(int& ID);

  // read oldest item without adjusting tail and count 
  TYPE* getFirstReadPtr(int& ID);

  //int getFirstWriteID();

  // (over)write element without adjusting head and count
  TYPE* getManualWritePtr(int startID);

  void setOffset(int offset, int& ID);
  
  // release locks
  void WriteUnlockElements(int ID, int nelements);
  void ReadUnlockElements(int ID, int nelements);
  void WriteUnlockElement(int ID);
  void ReadUnlockElement(int ID);
  

  // print head, tail and count pointers
  void Dump();

  // maximum number of items in buffer
  int  getSize();
  
  // actual number of written items in buffer
  int  getCount(); 

 private:

  // add elements to buffer
  int   AddBufferElement(TYPE DataElement);
  
  // clear element from buffer
  void  RemoveBufferElement(int ID);
  
  // clear the buffer
  void  RemoveElements();

  // set locks
  void ReadLockElement(int ID);
  void WriteLockElement(int ID);
  
  vector< BufferElement <TYPE> > itsBuffer;
  TYPE* itsElements;

  int itsHeadIdx;
  int itsTailIdx;
  int itsCount;
  int itsBlockCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
  pthread_cond_t  space_available;
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer(int nelements) :
    itsHeadIdx(0),
    itsTailIdx(0),
    itsCount(0),
    itsBlockCount(0)
{
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_cond_init (&data_available,  NULL);
  pthread_cond_init (&space_available, NULL);

  // create the cyclic buffer
  itsElements = new TYPE[nelements];
  for (int i=0; i< nelements; i++) {
    AddBufferElement(itsElements[i]);
  }
}

template<class TYPE>
CyclicBuffer<TYPE>::~CyclicBuffer()
{ 
  // clear the cyclic buffer
  RemoveElements();
  delete[] itsElements;
}

template<class TYPE>
int CyclicBuffer<TYPE>::AddBufferElement(TYPE DataElement)
{
  BufferElement<TYPE> elem(DataElement);

  pthread_mutex_lock(&buffer_mutex);

  // add the element at the back of the deque
  itsBuffer.push_back(elem);

  pthread_mutex_unlock(&buffer_mutex);

  // the ID will be the index in the deque
  return ((int)itsBuffer.size()-1);
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

  // clear the buffer
  itsBuffer.clear();

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getAutoWritePtr(int& ID)
{
  pthread_mutex_lock(&buffer_mutex);
  
  // wait until space becomes available
  while (itsBlockCount >= MAX_COUNT)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  // CONDITION: itsCount < MAX_COUNT
  ID = itsHeadIdx;
  WriteLockElement(ID);
  
  itsHeadIdx++;
  if (itsHeadIdx >= (int)itsBuffer.size()) {
    itsHeadIdx = 0;
  }
  itsBlockCount++;
  itsCount++;
  
  pthread_mutex_unlock(&buffer_mutex);

  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getAutoReadPtr(int& ID)
{
  
  pthread_mutex_lock(&buffer_mutex);
 
  // wait until enough elements are available
  while ((itsCount < MIN_COUNT)) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // CONDITION: itsCount >= MIN_COUNT
  ID = itsTailIdx;
  ReadLockElement(ID);

  itsTailIdx++;
  if (itsTailIdx >= (int)itsBuffer.size()) {
    itsTailIdx = 0;
  }
  itsCount --;
  pthread_mutex_unlock(&buffer_mutex);
  
  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getFirstReadPtr(int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (itsCount <= 0) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > 0
  
  ID = itsTailIdx;
  ReadLockElement(ID);
 
  pthread_mutex_unlock(&buffer_mutex);
  
  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getManualWritePtr(int startID)
{
  if ( (startID >= itsBuffer.size()) || (startID < 0))
  {
   LOG_TRACE_RTTI("CyclicBuffer::getWriteUserItem: ID has invalid value");
   return 0;
  } 

  pthread_mutex_lock(&buffer_mutex); 

  // lock the element
  WriteLockElement(startID);

  pthread_mutex_unlock(&buffer_mutex);

  return &itsBuffer[startID].itsElement;
}

template<class TYPE>
void CyclicBuffer<TYPE>::setOffset(int offset, int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until enough space becomes available
  while (itsCount - offset < MIN_COUNT)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  // CONDITION: itsCount - offset >= MIN_COUNT
  
  itsTailIdx += offset;
  if (itsTailIdx >= (int)itsBuffer.size()) {
    itsTailIdx -= (int)itsBuffer.size();
  }
  else if (itsTailIdx < 0) {
    itsTailIdx += (int)itsBuffer.size();
  }
  //itsReadIdx = itsTailIdx;
  ID = itsTailIdx;

  itsCount -= offset;
  itsBlockCount = itsCount;
   
  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteLock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElements(int ID, int nelements)
{
  for (int i=0; i<nelements; i++) {
    if (ID >= (int)itsBuffer.size()) {
      ID = 0;
    }
    itsBuffer[ID].itsRWLock.WriteUnlock();
    ID++;
  }
  pthread_mutex_lock(&buffer_mutex);
  
  // signal that data has become available 
  pthread_cond_broadcast(&data_available);

  pthread_mutex_unlock(&buffer_mutex);
  
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.WriteUnlock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadLockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadLock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElements(int ID, int nelements)
{

  for (int i=0; i<nelements; i++) {
    if (ID >= (int)itsBuffer.size()) {
      ID = 0;
    }
    itsBuffer[ID].itsRWLock.ReadUnlock();
    ID++;
  }
  
  pthread_mutex_lock(&buffer_mutex);
  
  itsBlockCount -= nelements;;
  
  // signal that space has become available
  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);

}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElement(int ID)
{
  itsBuffer[ID].itsRWLock.ReadUnlock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::Dump(void)
{
  pthread_mutex_lock(&buffer_mutex);
  
  cerr << "itsHeadIdx = " << itsHeadIdx << endl;
  cerr << "itsTailIdx = " << itsTailIdx << endl;
  cerr << "itsCount   = " << itsCount   << endl;

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
int CyclicBuffer<TYPE>::getSize(void)
{
  return (int)itsBuffer.size();
}

template<class TYPE>
int CyclicBuffer<TYPE>::getCount(void)
{
  return itsCount;
}



}

#endif
