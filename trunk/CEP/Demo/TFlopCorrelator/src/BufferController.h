//#  BufferController.h: template class to control a cyclic buffer.
//#
//#  Copyright (C) 2000, 2001
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


#ifndef TFLOPCORRELATOR_BUFFERCONTROLLER_H
#define TFLOPCORRELATOR_BUFFERCONTROLLER_H

#include <pthread.h>
#include "CyclicBuffer.h"


/* Main purpose of the BufferController class is to control a type independant
   cyclic buffer */


namespace LOFAR
{

template <class TYPE>
class BufferController
{
public:

  BufferController(int size);
  ~BufferController();
  
  TYPE* getBufferReadPtr();
  TYPE* getBufferWritePtr();

  TYPE* getFirstReadPtr(int* ID);
  TYPE* getUserReadPtr(uint offset);
  TYPE* getUserWritePtr(uint ID);
  
  void readyReading();
  void readyWriting();
  void dump();
  
  int getBufferSize(); // maximum number of elements in Cyclic Buffer
  int getBufferCount();// actual number of elements in Cyclic Buffer

private:
  
  // current read info
  TYPE* itsCurrentReadPtr;
  int itsCurrentReadID;
  
  // current write info
  TYPE* itsCurrentWritePtr;
  int itsCurrentWriteID;

  CyclicBuffer<TYPE*> itsBuffer;
  int itsBufferSize;

  TYPE* itsBufferItems;  
};
  
template<class TYPE>
BufferController<TYPE>::BufferController(int size)
  : itsBufferSize(size)  
{
  LOG_TRACE_FLOW("BufferController constructor");

  itsCurrentReadPtr = (TYPE*)0;
  itsCurrentReadID = -1;

  itsCurrentWritePtr = (TYPE*)0;
  itsCurrentWriteID = -1;

  // construct cyclic buffer
  itsBufferItems = new TYPE[size];
  for (int i = 0; i < itsBufferSize; i++) 
  {  
    itsBuffer.AddBufferItem(&itsBufferItems[i]);
  }
  itsBuffer.setWrittenBeforeReading(0); 
}

template<class TYPE>
BufferController<TYPE>::~BufferController()
{
  LOG_TRACE_FLOW("BufferController destructor");

  // clear the cyclic buffer
  itsBuffer.RemoveItems();
  delete[] itsBufferItems;
}

template<class TYPE>
TYPE* BufferController<TYPE>::getBufferReadPtr()
{
  if (itsCurrentReadPtr == 0) {
    itsCurrentReadPtr = itsBuffer.GetBufferReadPtr(&itsCurrentReadID);
  }
  return itsCurrentReadPtr; 
}

template<class TYPE>
TYPE*  BufferController<TYPE>::getBufferWritePtr()
{
  if (itsCurrentWritePtr == 0) {
    itsCurrentWritePtr = itsBuffer.GetBufferWritePtr(&itsCurrentWriteID);
  }
  return itsCurrentWritePtr; 
}

template<class TYPE>
TYPE* BufferController<TYPE>::getFirstReadPtr(int* ID)
{
  if (itsCurrentReadPtr ==0) {
    itsCurrentReadPtr = itsBuffer.GetFirstReadPtr(&itsCurrentReadID);
  }
  *ID = itsCurrentReadID; 
  return itsCurrentReadPtr; 
}

template<class TYPE>
TYPE* BufferController<TYPE>::getUserReadPtr(uint offset)
{
  if (itsCurrentReadPtr == 0) {
    itsCurrentReadPtr = itsBuffer.GetUserReadPtr(offset, &itsCurrentReadID);
  }
  return itsCurrentReadPtr; 
}

template<class TYPE>
TYPE* BufferController<TYPE>::getUserWritePtr(uint ID)
{
  if (itsCurrentWritePtr == 0) {
    itsCurrentWritePtr = itsBuffer.GetUserWritePtr(ID);
    itsCurrentWriteID = ID;
  }
  return itsCurrentWritePtr; 
}

template<class TYPE>
void BufferController<TYPE>::readyReading()
{
  if (itsCurrentReadID >= 0) 
  {
    itsBuffer.ReadUnlockItem(itsCurrentReadID); 
  }
  else
  {
    LOG_TRACE_RTTI("BufferController::readpointer not previously requested with getradPtr() function");
  }
  itsCurrentReadPtr = (TYPE*)0;
  itsCurrentReadID = -1;
}



template<class TYPE>
void BufferController<TYPE>::readyWriting()
{
  if (itsCurrentWriteID >= 0) 
  {
    itsBuffer.WriteUnlockItem(itsCurrentWriteID);
  }
  else {
    LOG_TRACE_RTTI("BufferController::writepointer not previously requested with getWritePtr() function");
  }
  
  itsCurrentWritePtr = (TYPE*)0;
  itsCurrentWriteID = -1;
}

template<class TYPE>
int BufferController<TYPE>::getBufferSize()
{
  return itsBuffer.getSize();
}

template<class TYPE>
int BufferController<TYPE>::getBufferCount()
{
  return itsBuffer.getCount();
}

template<class TYPE>
void BufferController<TYPE>::dump()
{
  itsBuffer.Dump();
}


}

#endif
