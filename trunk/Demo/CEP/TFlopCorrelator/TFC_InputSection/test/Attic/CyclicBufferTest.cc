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
  BufferController* bufcontrol;
} thread_args;


void* produce(void* argument)
{
  cout << "producer thread started" << endl;

  thread_args* arg = (thread_args*)argument;
  
  int blocksize = 1;

  timestamp_t ts[blocksize];
  for (int b=0; b< blocksize;b++) {
    ts[b].setStamp(0,b);
  }
  for (int i=0; i< 1000; i++) {
    
    arg->bufcontrol->writeElements(&ts,ts[0], blocksize, 1 );
    for (int j=0; j< blocksize;j++) {
      ts[j] += blocksize;
    }
  }

}

void* consume(void* argument)
{
  cout << "consumer thread started" << endl;

  thread_args* arg = (thread_args*)argument;

  timestamp_t ts(0,0);
  int invalidcount, seqid, blockid;
  int blocksize = 5;
  char data[8*blocksize];
  for (int i=0; i< 200; i++) {
    arg->bufcontrol->getElements(data,invalidcount, ts, blocksize);
    ts+=blocksize;

    for (int t=0; t<blocksize; t++) { 
      seqid = ((int*)&data[t*8])[0];
      blockid = ((int*)&data[t*8+4])[0];
      cout << seqid << "," << blockid << endl;  
    }
    cout << "invalid: " << invalidcount << endl;
    cout << endl;
  }
  
}


int main (int argc, const char** argv)
{ 
  try {

    // if (argc < 4) {
//       cout << "\nusage: CyclicBufferTest [buffersize] [readblocksize] [readblockoffset]\n" << endl;
//       exit (1);
//     }
    
//    // get buffersize
//    int buffersize = atoi(argv[1]);
//    int blocksize = atoi(argv[2]);
//    int offset = atoi(argv[3]); 
   

   // create Parameter Object
   ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 

   // create BufferController Object
   BufferController* BufControl = new BufferController(ps); 
    
   // start producing data thread
   pthread_t  producer;
   thread_args producerdata;
    
   producerdata.bufcontrol = BufControl;

   if (pthread_create(&producer, NULL, produce, &producerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }

   // start consuming data thread
   pthread_t consumer;
   thread_args consumerdata;

   consumerdata.bufcontrol = BufControl;

   if (pthread_create(&consumer, NULL, consume, &consumerdata) < 0)
   {
     perror("pthread_create");
     exit(1);
   }
  
   pthread_join(producer, NULL);
   pthread_join(consumer, NULL);

   delete BufControl;
   
     
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "Cyclic buffer OK" << endl;
  return 0;
}

