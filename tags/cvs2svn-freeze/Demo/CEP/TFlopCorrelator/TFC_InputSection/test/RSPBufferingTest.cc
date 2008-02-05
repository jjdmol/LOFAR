//#  RSPBufferingTest.cc: a test program for catching rsp data from ethernet link
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
#include <Transport/TH_Ethernet.h>
#include <string.h>
#include <Common/LofarLogger.h>


using namespace LOFAR;


typedef struct
{
  BufferController* bc;
  TH_Ethernet* con;
  int packetsize;
  int framesize;
  int npacketsinframe;
} producer_args;

typedef struct
{
  BufferController* bc;
  int nsubbands;
  int nsamples;
} consumer_args;



void* write(void* argument)
{
  cout << "bufferwriter thread started" << endl;
  
  producer_args* args = (producer_args*)argument;
  
  char recvframe[args->framesize];
  timestamp_t actualstamp, expectedstamp;
  int seqid, blockid;
  bool readnew  = true;
  bool firstloop = true;

  // define a block of dummy data
  SubbandType dummyblock[args->npacketsinframe];
  memset(dummyblock, 0, args->npacketsinframe*sizeof(SubbandType));

  while (1)
  {
    // catch frame from ethernet link
    if (readnew) {
      args->con->recvBlocking( (void*)recvframe, args->framesize, 0);
      seqid   = ((int*)&recvframe[8])[0];
      blockid = ((int*)&recvframe[12])[0];
    }
    actualstamp.setStamp(seqid ,blockid); 
    
    if (firstloop) {
      expectedstamp = actualstamp;
      firstloop = false;
    }
    
    if (actualstamp < expectedstamp) {
      // old packet received
      // do nothing
      //cout << "** old packets received ** " << expectedstamp << " : " << actualstamp << endl;
      readnew = true;
    }
    else if (actualstamp > expectedstamp) {
      // missed a packet so create a dummy
      args->bc->writeDummy((SubbandType*)dummyblock, expectedstamp, args->npacketsinframe);
      //cout << "** dummies created for timeblock " << expectedstamp << " to "<< expectedstamp+(args->npacketsinframe-1) << endl;
      readnew = false;
      expectedstamp += args->npacketsinframe;
    }
    else {
      //expected packet received  
      for (int i=0; i<args->npacketsinframe;i++) {
	args->bc->writeElements( (SubbandType*)&recvframe[i*args->packetsize+16], actualstamp);
	actualstamp++;
      }
      //cout << "expected packet received" << endl;
      readnew = true;
      expectedstamp += args->npacketsinframe;
    }
  } 
}

void* read(void* argument)
{
  cout << "buffer reader thread started" << endl;

  int invalidcount;
  timestamp_t ts;
  consumer_args* args = (consumer_args*)argument;

  vector<SubbandType*> subbandbuffer;
  for (int s=0; s < args->nsubbands; s++) {
    subbandbuffer.push_back(new SubbandType[args->nsamples]);
  }

  ts = args->bc->startBufferRead();
  cout << "reader started on stamp " << ts << endl;

 
  int seqid = 0;
  int packets = 0;
  int invalid = 0;
  while (1) {
    args->bc->getElements(subbandbuffer, invalidcount, ts, args->nsamples);
    packets += args->nsamples;
    if (invalidcount != 0) {
      invalid += invalidcount;
    }
    if (seqid != ts.getSeqId()) {
      seqid = ts.getSeqId();
      cout << "sequence "<< seqid << ", packets: " << packets << ", invalid: " << invalid << endl;
    }
    ts+=args->nsamples;
  }
}
 


int main (int argc, const char** argv)
{ 
  try {

  INIT_LOGGER("RSPBufferingTest");

   // create Parameter Object
   ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 
  
   // get ethernet params
   vector<string> interfaces = ps.getStringVector("Input.Interfaces");
   vector<string> srcMacs = ps.getStringVector("Input.SourceMacs");
   vector<string> dstMacs = ps.getStringVector("Input.DestinationMacs");
   
   // create TH_Ethernet object
   TH_Ethernet* connection = new TH_Ethernet(interfaces[0],
                                             srcMacs[0],
                                             dstMacs[0],
                                             0x000);

   // init connection
   connection->init();  

   // create BufferController Object
   BufferController BufControl(ps.getInt32("Input.CyclicBufferSize"), 
                               ps.getInt32("Data.NSubbands"));
    
   // start bufferwriter thread
   pthread_t  writer;
   producer_args writerdata;
    
   writerdata.bc = &BufControl;
   writerdata.con = connection;
   writerdata.npacketsinframe = ps.getInt32("Input.NPacketsInFrame");
   writerdata.packetsize = ps.getInt32("Input.SzEPApayload") + ps.getInt32("Input.SzEPAheader");
   writerdata.framesize = writerdata.packetsize *
                          writerdata.npacketsinframe;


   if (pthread_create(&writer, NULL, write, &writerdata) < 0)
   {
     perror("writer pthread_create");
     exit(1);
   }

   // start bufferreader thread
   pthread_t reader;
   consumer_args readerdata;

   readerdata.bc = &BufControl;
   readerdata.nsubbands = ps.getInt32("Data.NSubbands");
   readerdata.nsamples = ps.getInt32("Data.NSamplesToIntegrate");
   
   if (pthread_create(&reader, NULL, read, &readerdata) < 0)
   {
     perror("reader pthread_create");
     exit(1);
   }
  
   pthread_join(writer, NULL);
   pthread_join(reader, NULL);

   delete connection;
     
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "Test OK" << endl;
  return 0;
}

