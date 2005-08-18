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


using namespace LOFAR;


typedef struct
{
  BufferController* bc;
  TH_Ethernet* con;
  int packetsize;
  int npackets;
} thread_args;


void* write(void* argument)
{
  cout << "bufferwriter thread started" << endl;
  
  thread_args* args = (thread_args*)argument;
  
  int framesize = args->npackets * args->packetsize;
  char recvframe[framesize];
  timestamp_t actualstamp;
  int seqid, blockid;

  for (int t=0;t<10;t++)
  {
    // catch frame from ethernet link
    args->con->recvBlocking( (void*)recvframe, framesize, 0);
  
    // copy timestamps into cyclicbuffer
    for (int i=0; i<args->npackets; i++) {
      seqid   = ((int*)&recvframe[i*args->packetsize+8])[0];
      blockid = ((int*)&recvframe[i*args->packetsize+12])[0];
      actualstamp.setStamp(seqid ,blockid); 
      cout << actualstamp << endl;
      args->bc->writeElements(&actualstamp, actualstamp, 1, 0);
    }
  } 
}

void* read(void* argument)
{
  cout << "buffer reader thread started" << endl;

  thread_args* args = (thread_args*)argument;

  timestamp_t bufferstamp, startstamp;
  int invalidcount;
  
  //get first timestamp in buffer
  startstamp = args->bc->getFirstStamp();
  
  for (int t=0;t<500;t++) 
  {
    
    args->bc->getElements(&bufferstamp,
                           invalidcount, 
                           startstamp, 
                           1);
    
    cout << bufferstamp << endl;
    startstamp++; 
  }  
}
 


int main (int argc, const char** argv)
{ 
  try {

   // create Parameter Object
   ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 
  
   // get ethernet params
   vector<string> interfaces = ps.getStringVector("Input.Interfaces");
   vector<string> srcMacs = ps.getStringVector("Input.SourceMacs");
   vector<string> dstMacs = ps.getStringVector("Input.DestinationMacs");
   
   // create TH_Ethernet object
   TH_Ethernet* connection = new TH_Ethernet(interfaces[0],
                                             dstMacs[0],
                                             srcMacs[0],
                                             0x000);

   // init connection
   connection->init();  

   // create BufferController Object
   BufferController BufControl(ps);
   BufControl.overwritingAllowed(false);
    
   // start bufferwriter thread
   pthread_t  writer;
   thread_args writerdata;
    
   writerdata.bc = &BufControl;
   writerdata.con = connection;
   writerdata.packetsize = ps.getInt32("Input.SzEPApayload") + ps.getInt32("Input.SzEPAheader");
   writerdata.npackets = ps.getInt32("Input.NPacketsInFrame");

   if (pthread_create(&writer, NULL, write, &writerdata) < 0)
   {
     perror("writer pthread_create");
     exit(1);
   }

   // start bufferreader thread
   pthread_t reader;
   thread_args readerdata;

   readerdata.bc = &BufControl;

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

