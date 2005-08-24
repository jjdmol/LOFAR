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
#define MIN_COUNT   10

// maximum of written elements in buffer to avoid that 
// writeptr is catching up readptr
#define MAX_COUNT  ((int)itsBuffer.size()-10)    

namespace LOFAR
{

class BufferIndex 
{
 public:

  BufferIndex(const int maxidx = 0);
  
  void operator+= (int increment);
  void operator++ (int); 
  void operator+ (int increment);
  void operator-= (int decrement);
  void operator-- (int); 
  int operator- (BufferIndex& other);
  bool operator== (BufferIndex& other);
  
  int getIndex();

 private:
  int itsIndex;
  int itsMaxIndex;
  void checkIndex();
};
 

inline BufferIndex::BufferIndex(int maxidx) :
  itsIndex(0),
  itsMaxIndex(maxidx)
{
}

inline void BufferIndex::checkIndex()
{
  if (itsIndex >= itsMaxIndex) {
    itsIndex -= itsMaxIndex;
  }
  else if (itsIndex < 0) {
    itsIndex += itsMaxIndex;
  }
}

inline int BufferIndex::getIndex()
{
  return itsIndex;
}

inline void BufferIndex::operator+= (int increment)
{
  itsIndex += increment;
  checkIndex();
}

inline void BufferIndex::operator++ (int)
{
  itsIndex++;
  checkIndex();
}

inline void BufferIndex::operator+ (int increment)
{
  itsIndex += increment;
  checkIndex();
}

inline void BufferIndex::operator-= (int decrement)
{
  itsIndex -= decrement;
  checkIndex();
}

inline void BufferIndex::operator-- (int)
{
  itsIndex--;
  checkIndex();
}

inline int BufferIndex::operator- (BufferIndex& other)
{
  int increment = itsIndex-other.itsIndex;  
  if (increment < 0) {
    return increment + itsMaxIndex;
  }
  return increment; 
}

inline bool BufferIndex::operator== (BufferIndex& other)
{
  return (itsIndex == other.itsIndex);
}

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

  // read newest item, throw older items away
  TYPE* CyclicBuffer<TYPE>::getNewestReadPtr(int &ID);

  // (over)write element without adjusting head and count
  TYPE* getManualWritePtr(int startID);

  // move tail and adjust count
  void setOffset(int offset, int& ID);
  
  // allow overwriting elements which haven't been read
  void setOverwritingAllowed(bool allowed);

  // unlock a writelocked block of elements
  void WriteUnlockElements(int ID, int nelements);
  // unlock a readlocked block of elements
  void ReadUnlockElements(int ID, int nelements);
  // unlock a writelocked element
  void WriteUnlockElement(int ID);
  // unlock a readlocked element
  void ReadUnlockElement(int ID);
  
  // dump head, tail and count of cyclic buffer 
  void Dump();

  // maximum number of elements in buffer
  int  getSize();
  
  // actual number of elements in buffer
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

  BufferIndex itsHeadIdx;  // writepointer
  BufferIndex itsTailIdx;  // readpointer

  BufferIndex itsOldHead;
  BufferIndex itsOldTail;
  
  // permission to overwrite previous written elements
  bool itsOverwritingAllowed;

  pthread_mutex_t buffer_mutex;    // lock/unlock shared data
  pthread_cond_t  data_available;  // 'buffer not empty' trigger
  pthread_cond_t  space_available; // 'buffer not full' trigger
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer(int nelements) :
     itsOverwritingAllowed(true),
     itsHeadIdx(nelements),
     itsTailIdx(nelements),
     itsOldHead(nelements),
     itsOldTail(nelements)
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

  // add the element at the back
  itsBuffer.push_back(elem);

  pthread_mutex_unlock(&buffer_mutex);

  // the ID will be the index
  return ((int)itsBuffer.size()-1);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveBufferElement(int ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // erase the element at the specified position
  itsBuffer.erase(ID);

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveElements(void)
{
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
  while ((itsHeadIdx - itsOldTail >= MAX_COUNT) && !itsOverwritingAllowed)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  ID = itsHeadIdx.getIndex();
  WriteLockElement(ID);

  itsHeadIdx++;

  // if allowed, overwrite previous written elements
  if ((itsHeadIdx == itsTailIdx) && itsOverwritingAllowed) {
    // push tail 1 element forward
    itsTailIdx++;
    itsOldTail++;
  }
  
  pthread_mutex_unlock(&buffer_mutex);

  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getAutoReadPtr(int& ID)
{
  
  pthread_mutex_lock(&buffer_mutex);

  // overwriting not allowed when reading is started
  itsOverwritingAllowed = false;
  
  // wait until enough elements are available
  while (itsOldHead-itsTailIdx < MIN_COUNT) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  ID = itsTailIdx.getIndex();
  ReadLockElement(ID);

  itsTailIdx++;
  
  pthread_mutex_unlock(&buffer_mutex);
  
  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getFirstReadPtr(int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (itsOldHead-itsTailIdx < 1) 
  {
    cout<<"getFirstReadPtr"<<endl;
    Dump();
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > 0
  
  ID = itsTailIdx.getIndex();
  ReadLockElement(ID);
 
  pthread_mutex_unlock(&buffer_mutex);
  
  return &itsBuffer[ID].itsElement;
}

template<class TYPE>
TYPE* CyclicBuffer<TYPE>::getNewestReadPtr(int &ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (itsOldHead-itsTailIdx < 1) 
  {
    cout<<"getNewestReadPtr"<<endl;
    Dump();
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > 0
  
  // This method is called before the first read,
  // so it is allowed to move the tails
  BufferIndex bid = itsOldHead;
  bid -= 1;
  ID = bid.getIndex();
  ReadLockElement(ID);
  itsTailIdx = bid;
  itsOldTail = bid;
 
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

  // wait until enough data becomes available
  while (itsOldHead-itsTailIdx-offset < MIN_COUNT)
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // This method is called when there is no reader,
  // so tail == oldTail
  itsTailIdx += offset;
  itsOldTail = itsTailIdx;
  ID = itsTailIdx.getIndex();
   
  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteLockElement(int ID)
{
#define DO_LOCKING_NOT
#ifdef DO_LOCKING
  itsBuffer[ID].itsRWLock.WriteLock();
#endif
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadLockElement(int ID)
{
#ifdef DO_LOCKING
  itsBuffer[ID].itsRWLock.ReadLock();
#endif
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElement(int ID)
{
#ifdef DO_LOCKING
  itsBuffer[ID].itsRWLock.WriteUnlock();
#endif
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElement(int ID)
{
#ifdef DO_LOCKING
  itsBuffer[ID].itsRWLock.ReadUnlock();
#endif
}


template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElements(int ID, int nelements)
{
  for (int i=0; i<nelements; i++) {
    if (ID >= (int)itsBuffer.size()) {
      ID = 0;
    }
#ifdef DO_LOCKING
    itsBuffer[ID].itsRWLock.WriteUnlock();
#endif
    ID++;
  }
  pthread_mutex_lock(&buffer_mutex);
  
  // synchronize writepointers
  itsOldHead = itsHeadIdx;

  // signal that data has become available 
  pthread_cond_broadcast(&data_available);

  pthread_mutex_unlock(&buffer_mutex);
  
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElements(int ID, int nelements)
{

  for (int i=0; i<nelements; i++) {
    if (ID >= (int)itsBuffer.size()) {
      ID = 0;
    }
#ifdef DO_LOCKING
    itsBuffer[ID].itsRWLock.ReadUnlock();
#endif
    ID++;
  }
  
  pthread_mutex_lock(&buffer_mutex);
  
  // synchronize readpointers
  itsOldTail = itsTailIdx;
  
  // signal that space has become available
  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);

}


template<class TYPE>
void CyclicBuffer<TYPE>::setOverwritingAllowed(bool allowed)
{
  itsOverwritingAllowed = allowed;
}

template<class TYPE>
void CyclicBuffer<TYPE>::Dump(void)
{
  pthread_mutex_lock(&buffer_mutex);
  
  cerr << "itsHeadIdx = " << itsHeadIdx.getIndex() << endl;
  cerr << "itsOldHead = " << itsOldHead.getIndex() << endl;
  cerr << "itsTailIdx = " << itsTailIdx.getIndex() << endl;
  cerr << "itsOldTail = " << itsOldTail.getIndex() << endl;
  cerr << "itsCount   = " << getCount()  << endl;

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
  return itsOldHead - itsTailIdx;
}



}

#endif
