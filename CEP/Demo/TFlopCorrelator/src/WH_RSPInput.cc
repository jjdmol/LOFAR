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

// Application specific includes
#include <WH_RSPInput.h>
#include <DH_Sync.h>
#include <DH_RSP.h>

using namespace LOFAR;


void* WriteToBufferThread(void* arguments)
{
  thread_args* args = (thread_args*)arguments;
  BufferController<DataType>* databuffer = args->databuffer;
  BufferController<MetaDataType>* metadatabuffer = args->metadatabuffer;
  TH_Ethernet* connection = args->connection;
  int framesize = args->framesize;
  int packetsinframe = args->packetsinframe;

  char recvframe[9000];
  int seqid, blockid, itemid, offset;
  timestamp_t actualstamp, nextstamp;
  DataType* dataptr;
  MetaDataType* metadataptr;
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
      
      // read oldest item in cyclic buffer
      metadataptr = (MetaDataType*)metadatabuffer->getFirstReadPtr(&itemid);
      
      // subtract timestamps to find offset
      offset = actualstamp - metadataptr->timestamp;
      
      // determine itemid to be written
      itemid += offset;
      
      // release readlock
      metadatabuffer->readyReading();
   
      // overwrite data and invalid flag
      metadataptr = (MetaDataType*)metadatabuffer->getUserWritePtr(itemid);   
      dataptr = (DataType*)databuffer->getUserWritePtr(itemid);
      if (metadataptr != 0 && dataptr != 0) {
        memcpy(dataptr->data, recvframe, framesize);
        metadataptr->invalid = 0;
      }      
     
      // release writelocks
      databuffer->readyWriting();
      metadatabuffer->readyWriting();
      
      // read new frame in next loop
      readnew = true;   
    }
    else if (nextstamp + (packetsinframe - 1) < actualstamp) {
      // missed a packet so set invalid flag and missing timestamp
      metadataptr = (MetaDataType*)metadatabuffer->getBufferWritePtr(); 
      metadataptr->invalid = 1;
      metadataptr->timestamp = nextstamp;

      // call getBufferWritePtr to increase the writepointer but
      // don't write data because we have none
      (DataType*)databuffer->getBufferWritePtr();   
     
      // release writelocks
      databuffer->readyWriting();
      metadatabuffer->readyWriting();
      
      // read same frame again in next loop
      readnew = false;
    } 
    else {
      // expected packet received so write data into buffer
      dataptr = (DataType*)databuffer->getBufferWritePtr();
      metadataptr = (MetaDataType*)metadatabuffer->getBufferWritePtr();      
      memcpy(dataptr->data, recvframe, framesize);
      metadataptr->invalid = 0;
      metadataptr->timestamp = actualstamp;      
     
      // release writelocks
      databuffer->readyWriting();
      metadatabuffer->readyWriting();
     
      // read new frame in next loop
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
  itsMetaDataBuffer = new BufferController<MetaDataType>(100); 
  
   // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Sync("DH_Sync"));

  // create outgoing dataholder holding the delay controlled RSP data
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out"));
 
}


WH_RSPInput::~WH_RSPInput() 
{
  delete itsDataBuffer;
  delete itsMetaDataBuffer;
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
  // start up writer thread
  writerinfo.databuffer = itsDataBuffer;
  writerinfo.metadatabuffer = itsMetaDataBuffer;
  writerinfo.connection = itsInputConnection;
  writerinfo.framesize = itsSzRSPframe;
  writerinfo.packetsinframe = itsNpackets;
  writerinfo.stopthread = false;
  
  if (pthread_create(&writerthread, NULL, WriteToBufferThread, &writerinfo) < 0)
  {
    perror("writer thread creation failure");
    exit(1);
  }
}

void WH_RSPInput::process() 
{ 
  DH_Sync* inDHp;
  DH_RSP* outDHp;
  DataType* dataptr;
  MetaDataType* metadataptr;
  timestamp_t syncstamp;
  int seqid, blockid;

  // get delay information
  inDHp = (DH_Sync*)getDataManager().getInHolder(0);
  inDHp->getNextMainBeat(seqid, blockid);
  syncstamp.setStamp(seqid, blockid);
  
  // add delay control code here

  // get meta data from cyclic buffer 
  metadataptr = itsMetaDataBuffer->getBufferReadPtr();
  
  // get data from cycclic buffer
  dataptr = itsDataBuffer->getBufferReadPtr();

  // write flag to outgoing dataholder
  outDHp->setFlag(metadataptr->invalid);

  // write data to outgoing dataholder
  memcpy(outDHp->getBuffer(), dataptr->data, itsSzRSPframe);
}

void WH_RSPInput::postprocess()
{
  // stop writer thread
  writerinfo.stopthread = true;
}

void WH_RSPInput::dump() 
{
}
