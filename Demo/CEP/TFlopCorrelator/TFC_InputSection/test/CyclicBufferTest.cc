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
#include <Common/hexdump.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;


typedef struct
{
  BufferController* bc;
} thread_args;


void* produce(void* argument)
{
  cout << "producer thread started" << endl;

  thread_args* arg = (thread_args*)argument;
  BufferController* bc = arg->bc;

  
  timestamp_t ts(0,0);
  for (int i=0; i< 120; i++) {
    bc->writeElements(&ts,ts, 1, 0 );
    ts++;
  }

}

void* consume(void* argument)
{
  cout << "consumer thread started" << endl;

  thread_args* arg = (thread_args*)argument;
  BufferController *bc = arg->bc;

  int blocksize = 12;
  timestamp_t ts(0,0);
  timestamp_t data[blocksize];
  int invalidcount;
  
  for (int i=0; i< 10; i++) {
    bc->getElements(data,invalidcount, ts, blocksize);
    ts+=blocksize;
    
    for (int t=0; t<blocksize; t++) { 
      cout << data[t].getSeqId() << "," << data[t].getBlockId() << endl;
    }
    cout << "invalid: " << invalidcount << endl;
    cout << endl;
  }
  
}



int main (int argc, const char** argv)
{ 
  try {

   // create Parameter Object
   ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 

   // create BufferController Object
   //BufferController* BufControl = new BufferController(ps); 
   BufferController BufControl(ps);
    
   // start producing data thread
   pthread_t  producer;
   thread_args producerdata;
    
   producerdata.bc = &BufControl;

   if (pthread_create(&producer, NULL, produce, &producerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }

   // start consuming data thread
   pthread_t consumer;
   thread_args consumerdata;

   consumerdata.bc = &BufControl;

   if (pthread_create(&consumer, NULL, consume, &consumerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }
  
   pthread_join(producer, NULL);
   pthread_join(consumer, NULL);
   
     
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "Cyclic buffer OK" << endl;
  return 0;
}

