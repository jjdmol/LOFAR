//#  WH_RSPInput.cc: Catch RSP ethernet frames and place them in buffer
//#
//#
//#  Copyright (C) 2002-2005
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

// General includes
#include <Common/LofarLogger.h>

#include <CEPFrame/DataManager.h>
//#include <tinyCEP/Profiler.h>

// Application specific includes
#include <WH_RSPInput.h>
#include <DH_RSP.h>

using namespace LOFAR;


void* WriteToBufferThread(void* arguments)
{
 cout << "writing thread started" << endl;
 thread_args* args = (thread_args*)arguments;
 
 BufferController<DataType>* databuffer = args->databuffer;
 BufferController<FlagType>* flagbuffer = args->flagbuffer;
 TH_Ethernet* connection = args->connection;
 int framesize = args->framesize;
 int packetsinframe = args->packetsinframe;

 char recvframe[9000];
 int seqid, blockid;
 TimeStamp actualstamp, nextstamp;
 DataType* dataptr;
 FlagType* flagptr;
 bool readnew = true;
 bool firstloop = true;

 while(1) {
   
   // check stop condition
   if (args->stopthread == true) {
     pthread_exit(NULL);
   }
   
   // catch a frame from input connection
   if (readnew) {
     connection->recvBlocking( (void*)recvframe, framesize, 0);
   }
   
   // get the actual timestamp
   seqid   = ((int*)&recvframe[6])[0];
   blockid = ((int*)&recvframe[10])[0];
   actualstamp.setStamp(seqid ,blockid);

   if (firstloop) {
    // set nextstamp equal to actualstamp
    nextstamp.setStamp(seqid, blockid);
    firstloop = false;
   }

   if (actualstamp < nextstamp) {
     // old packet received 
     // if exists, overwrite its previous created dummy
     
     readnew = true;   
   }
   else if (nextstamp + (packetsinframe - 1) < actualstamp) {
     // missed a packet so write dummy into buffer 
     

     readnew = false;
   } 
   else {
     // expected packet received so write data into buffer
     dataptr = (DataType*)databuffer->getBufferWritePtr();
     flagptr = (FlagType*)flagbuffer->getBufferWritePtr();
     
     memcpy(dataptr->item, recvframe, framesize);
     flagptr->item = 0;      
     databuffer->readyWriting();
     flagbuffer->readyWriting();
     readnew = true;
   } 
 }
}

WH_RSPInput::WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
                         const string device,
                         const string srcMAC,
                         const string destMAC)
  : WorkHolder (1, 1, name, "WH_RSPInput"),
    itsKVM (kvm),
    itsDevice(device),
    itsSrcMAC(srcMAC),
    itsDestMAC(destMAC)
{
  // number of EPA packets per RSP frame
  itsNpackets = kvm.getInt("NoPacketsInFrame", 8);
  
  // size of an EPA packet in bytes 
  int sizeofpacket   = ( kvm.getInt("polarisations",2) * 
                          sizeof(complex<int16>) * 
                          kvm.getInt("NoRSPBeamlets", 92)
                       ) + 
                       kvm.getInt("SzEPAheader", 14); 
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNpackets * sizeofpacket;

  // create raw ethernet interface to catch incoming RSP data
  itsInputConnection = new TH_Ethernet(itsDevice, itsSrcMAC, itsDestMAC, 0x000 );

  // use  cyclic buffers to hold the rsp data and valid/invalid flag
  itsDataBuffer = new BufferController<DataType>(100); 
  itsFlagBuffer = new BufferController<FlagType>(100); 
  
   // create incoming dataholder holding the delay information 
  //getDataManager().addInDataHolder(0, new DH_Delay("DH_delay"));

  // create outgoing dataholder holding the delay controlled RSP data
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out"));
 
}


WH_RSPInput::~WH_RSPInput() 
{
  delete itsDataBuffer;
  delete itsFlagBuffer;
  delete itsInputConnection;
}


WorkHolder* WH_RSPInput::construct(const string& name,
                                   const KeyValueMap kvm,
                                   const string device,
                                   const string srcMAC,
                                   const string destMAC)
{
  return new WH_RSPInput(name, kvm, device, srcMAC, destMAC);
}


WH_RSPInput* WH_RSPInput::make(const string& name)
{
  return new WH_RSPInput(name, itsKVM, itsDevice, itsSrcMAC, itsDestMAC);
}


void WH_RSPInput::preprocess()
{
  // writer thread information
  writerinfo.databuffer = itsDataBuffer;
  writerinfo.flagbuffer = itsFlagBuffer;
  writerinfo.connection = itsInputConnection;
  writerinfo.framesize = itsSzRSPframe;
  writerinfo.packetsinframe = itsNpackets;
  writerinfo.stopthread = false;
  
  // start writer thread 
  if (pthread_create(&writerthread, NULL, WriteToBufferThread, &writerinfo) < 0)
  {
    perror("writer thread creation failure");
    exit(1);
  }
}


void WH_RSPInput::postprocess()
{
  // stop writer thread
  writerinfo.stopthread = true;
}


void WH_RSPInput::process() 
{ 
  //DH_Delay* inDHp;
  DH_RSP* outDHp;
  DataType* dataptr;
  FlagType* flagptr;

  // get delay information
  //inDHp = (DH_Delay*)getDataManager().getInHolder(0);
  
  // add delay control code here

  // get flag from cyclic buffer 
  flagptr = itsFlagBuffer->getBufferReadPtr();
  
  // get data from cycclic buffer
  dataptr = itsDataBuffer->getBufferReadPtr();

  // write flag to outgoing dataholder
  outDHp->setFlag(flagptr->item);

  // write data to outgoing dataholder
  memcpy(outDHp->getBuffer(), dataptr->item, itsSzRSPframe);
}

void WH_RSPInput::dump() 
{
}
