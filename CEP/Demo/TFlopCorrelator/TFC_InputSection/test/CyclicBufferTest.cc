//#  CyclicBufferTest.cc: a test program for the BufferController class
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include "../src/BufferController.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

using namespace LOFAR;

typedef struct
{
  char data[100];
} dataformat;


typedef dataformat BufferType;

typedef struct
{
  BufferController<BufferType>* buffer;
  int size;
} thread_args;


void* produce(void* arguments)
{
  cout << "producer thread started" << endl;

  thread_args* args = (thread_args*)arguments;
  BufferController<BufferType>* buffer = args->buffer;
  int size = args->size;
  BufferType *pelement;
  char buf[100];
  int id;

  for (int i=0;i<size;i++) {
    pelement = buffer->getBufferWritePtr();
    sprintf(pelement->data, "element %d", i);
    buffer->readyWriting();
    buffer->dump();
  }
}

void* consume(void* arguments)
{
  cout << "consumer thread started" << endl;

  thread_args* args = (thread_args*)arguments;
  BufferController<BufferType>* buffer = args->buffer;
  int size = args->size;
  BufferType *pelement;

  for (int i=0;i<size;i++) {
    pelement = buffer->getBufferReadPtr();
    buffer->readyReading();
    buffer->dump();
  }
}


int main (int argc, const char** argv)
{ 
  try {

    
    if (argc < 2) {
      cout << "\nusage: CyclicBufferTest [buffersize]\n" << endl;
      exit (1);
    }
    
   // get buffersize
   int size = atoi(argv[1]);

   // create BufferController Object
    BufferController<BufferType>* DataBuffer = new  BufferController<BufferType>(size); 
    
   // start producing data thread
   pthread_t producer;
   thread_args producerdata;
    
   producerdata.buffer = DataBuffer;
   producerdata.size = size;

   if (pthread_create(&producer, NULL, produce, &producerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }
   
   // start consuming data thread
   pthread_t consumer;
   thread_args consumerdata;

   consumerdata.buffer = DataBuffer;
   consumerdata.size = size;

   if (pthread_create(&consumer, NULL, consume, &consumerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }
  
   pthread_join(producer, NULL);
   pthread_join(consumer, NULL);
   
   
   delete DataBuffer;
     
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "Cyclic buffer OK" << endl;
  return 0;
}

