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
  int nsubbands;
} thread_args;


void* produce(void* argument)
{
  cout << "producer thread started" << endl;

  thread_args* arg = (thread_args*)argument;
  BufferController* bc = arg->bc;
  int nsubbands = arg->nsubbands;
  
  timestamp_t ts(0,0);
  SubbandType data[nsubbands];
  
  //Fill the subbanddata
  for (int i=0; i<nsubbands; i++) {
    data[i].Xpol = makei16complex(1,1);
    data[i].Ypol = makei16complex(0,0);
  }

  while (1) {
    bc->writeElements(data,ts);
    ts++;   
  }
}

void* consume(void* argument)
{
  cout << "consumer thread started" << endl;
  sleep(1); 
  
  thread_args* arg = (thread_args*)argument;
  BufferController *bc = arg->bc;
  int nsubbands = arg->nsubbands;
  int nelements = 10;
  int invalidcount;
  timestamp_t ts(0,0);

  // create databuffer
  vector<SubbandType*> databuffer;
  for (int s=0; s<nsubbands; s++) {
    databuffer.push_back(new SubbandType[nelements]);
  }

  ts = bc->startBufferRead();

  while (1) {
    bc->getElements(databuffer, invalidcount, ts, nelements);
    for (int i=0; i<nsubbands; i++) {
      for (int j=0; j<nelements; j++) {
        cout <<  databuffer[i][j].Xpol << "," << databuffer[i][j].Ypol << endl;
      }
    }
    ts+=nelements;
  }
}



int main (int argc, const char** argv)
{ 
  try {

   // create Parameter Object
   ACC::APS::ParameterSet ps("TFlopCorrelator.cfg");
   
   int nsubbands = 4;
   // create BufferController Object
   BufferController BufControl(1000, nsubbands);
    
   // start producing data thread
   pthread_t  producer;
   thread_args producerdata;
    
   producerdata.bc = &BufControl;
   producerdata.nsubbands = nsubbands;

   if (pthread_create(&producer, NULL, produce, &producerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }

   // start consuming data thread
   pthread_t consumer;
   thread_args consumerdata;

   consumerdata.bc = &BufControl;
   consumerdata.nsubbands = nsubbands;

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

