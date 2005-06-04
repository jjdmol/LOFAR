//#  BufferController.h: template class buffercontrol
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


#ifndef STATIONCORRELATOR_BUFFERCONTROLLER_H
#define STATIONCORRELATOR_BUFFERCONTROLLER_H

#include <pthread.h>
#include "CyclicBuffer.h"


/* Main purpose of the BufferController class is to control a type independant
   cyclic buffer 
*/


namespace LOFAR
{

template <class TYPE>
class BufferController
{
public:

  BufferController(int size);
  ~BufferController();
  
  TYPE& getInHolder();
  TYPE& getOutHolder();
  

  void readyWithInHolder();
  void readyWithOutHolder();
  
  void preprocess();
  void postprocess();

  int getBufferSize();
  int getBufferCount();

private:
  
  // Starts a reader thread
  void read();
  
  // Starts a writer thread
  void write();
   
  // Start function for reader thread
  static void* startReaderThread(void* thread_arg);
  
  // Start function for writer thread
  static void* startWriterThread(void* thread_arg);

  typedef struct thread_args{
    pthread_mutex_t mutex;
    bool            stopThread;
    CyclicBuffer<TYPE> *buffer;
  }thread_data;


  pthread_t itsInHolder;
  pthread_t itsOutHolder;
  thread_data itsInThreadData;
  thread_data itsOutThreadData;

  TYPE itsCurrentInData;
  int itsCurrentInID;
  
  TYPE itsCurrentOutData;
  int itsCurrentOutID;

  CyclicBuffer<TYPE> itsBuffer;
  int itsBufferSize;
  
};
  
template<class TYPE>
BufferController<TYPE>::BufferController(int size)  
{
  LOG_TRACE_FLOW("BufferController constructor");

  int instatus = pthread_mutex_init(&itsInThreadData.mutex, NULL);
  DBGASSERTSTR(instatus == 0, "Init mutex failed");

  int outstatus = pthread_mutex_init(&itsOutThreadData.mutex, NULL);
  DBGASSERTSTR(outstatus == 0, "Init mutex failed");

  itsInThreadData.stopThread = true;
  itsCurrentInData = 0;
  itsCurrentInID = -1;

  itsOutThreadData.stopThread = true;
  itsCurrentOutData = 0;
  itsCurrentOutID = -1;
}

template<class TYPE>
BufferController<TYPE>::~BufferController()
{
  LOG_TRACE_FLOW("BufferController destructor");
  pthread_mutex_lock(&itsInThreadData.mutex);
  if (itsInThreadData.stopThread == false) {
    itsInThreadData.stopThread = true;  // Causes thread to exit
  }
  pthread_mutex_unlock(&itsInThreadData.mutex);

  pthread_mutex_lock(&itsOutThreadData.mutex);
  if (itsOutThreadData.stopThread == false) {
    itsOutThreadData.stopThread = true;  // Causes thread to exit
  }
  pthread_mutex_unlock(&itsOutThreadData.mutex);
}

template<class TYPE>
TYPE& BufferController<TYPE>::getInHolder()
{
  if (itsCurrentInData == 0) {
    itsCurrentInData = itsBuffer.GetReadDataItem(&itsCurrentInID);
  }
  return itsCurrentInData; 
}

template<class TYPE>
TYPE&  BufferController<TYPE>::getOutHolder()
{
  if (itsCurrentOutData == 0) {
    itsBuffer.GetWriteLockedDataItem(&itsCurrentOutID);
  }
  return itsCurrentOutData; 
}

template<class TYPE>
void BufferController<TYPE>::readyWithInHolder()
{
  if (itsCurrentInID >= 0) 
  {
    read(); 
    itsBuffer.ReadUnlockElement(itsCurrentInID); 
  }
  else
  {
    LOG_TRACE_RTTI("BufferController::readyWithHolder() holder not previously requested with getHolder() function");
  }
  itsCurrentInData = 0;
  itsCurrentInID = -1;
}

template<class TYPE>
void BufferController<TYPE>::readyWithOutHolder()
{
  if (itsCurrentOutID >= 0) 
  {
    write();
    itsBuffer.WriteUnlockElement(itsCurrentOutID);
  }
  else {
    LOG_TRACE_RTTI("BufferController::readyWithHolder() holder not previously requested with getHolder() function");
  }
  
  itsCurrentOutData = 0;
  itsCurrentOutID = -1;
}

template<class TYPE>
void BufferController<TYPE>::preprocess()
{  
  TYPE element;
  for (int i = 0; i < itsBufferSize; i++)   // Fill buffer
  {
    itsBuffer.AddBufferElement(element);
    cout << "Add element " << element << endl;
  } 
}

template<class TYPE>
void BufferController<TYPE>::postprocess()
{
}

template<class TYPE>
void* BufferController<TYPE>::startReaderThread(void* thread_arg)
{
  LOG_TRACE_RTTI_STR("In reader thread ID " << pthread_self());
  thread_data* data = (thread_data*)thread_arg;
  
  while (1)
  {
    pthread_mutex_lock(&data->mutex); // check stop condition
    if (data->stopThread == true)
    {
      pthread_mutex_unlock(&data->mutex);
      LOG_TRACE_RTTI_STR("Reader thread " << pthread_self() << " exiting");
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&data->mutex);
    int id;
    LOG_TRACE_RTTI_STR("Thread " << pthread_self() << " attempting to read");

    data->buffer->GetWriteLockedDataItem(&id);
    data->buffer->WriteUnlockElement(id);
  }
}

template<class TYPE>
void* BufferController<TYPE>::startWriterThread(void* thread_arg)
{
  LOG_TRACE_RTTI_STR("In writer thread ID " << pthread_self());
  thread_data* data = (thread_data*)thread_arg;

  while (1)
  {
    pthread_mutex_lock(&data->mutex);
    if (data->stopThread == true) // check stop condition
    {
      pthread_mutex_unlock(&data->mutex);
      LOG_TRACE_RTTI_STR("Writer thread " << pthread_self() << " exiting");
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&data->mutex);
    int id;
    LOG_TRACE_RTTI_STR("Thread " << pthread_self() << " attempting to write");
    data->buffer->GetReadDataItem(&id);
    data->buffer->ReadUnlockElement(id);
  }
}

template<class TYPE>
void BufferController<TYPE>::read()
{
  pthread_mutex_lock(&itsInThreadData.mutex);
  if (itsInThreadData.stopThread == true)
  {                                          
    itsInThreadData.buffer = &itsBuffer;
    itsInThreadData.stopThread = false;
    pthread_create(&itsInHolder, NULL, startReaderThread, &itsInThreadData);
    LOG_TRACE_RTTI_STR("Reader thread " << itsInHolder << " created");
  }

  pthread_mutex_unlock(&itsInThreadData.mutex);
}

template<class TYPE>
void BufferController<TYPE>::write()
{
  pthread_mutex_lock(&itsOutThreadData.mutex);

  if (itsOutThreadData.stopThread == true)
  {    
    itsOutThreadData.buffer = &itsBuffer;
    itsOutThreadData.stopThread = false;
    pthread_create(&itsOutHolder, NULL, startWriterThread, &itsOutThreadData);
    LOG_TRACE_RTTI_STR("Writer thread " << itsOutHolder << " created");
  }

  pthread_mutex_unlock(&itsOutThreadData.mutex);
}

template<class TYPE>
int BufferController<TYPE>::getBufferSize()
{
  return itsBuffer.GetSize();
}

template<class TYPE>
int BufferController<TYPE>::getBufferCount()
{
  return itsBuffer.GetCount();
}


}

#endif
