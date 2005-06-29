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

// Application specific includes
#include <WH_RSPInput.h>
#include <TFC_Interface/DH_Sync.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_RSPSync.h>
      

using namespace LOFAR;


void* WriteToBufferThread(void* arguments)
{
  thread_args* args = (thread_args*)arguments;
  BufferController<dataType>* databuffer = args->databuffer;
  TH_Ethernet* connection = args->connection;
  TinyDataManager* datamanager = args->datamanager;
  int framesize = args->framesize;
  int packetsinframe = args->packetsinframe;
  bool syncmaster = args->syncmaster;

  char recvframe[9000];
  int seqid, blockid, itemid, offset;
  timestamp_t actualstamp, nextstamp;
  dataType* dataptr;
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

    if (!syncmaster) {
    // we are a slave so read the syncstamp
      if (firstloop) {
        // this is the first time
        // we need to force a read, because this inHolder has a lower data rate
        // if the rate difference is 1000, the workholder will read for
        // the first time in the 1000th runstep.
        datamanager->getInHolder(1);
        datamanager->readyWithInHolder(1);
        firstloop = false;
      }      
      DH_RSPSync* dhp = (DH_RSPSync*)datamanager.getInHolder(1);
      nextstamp = dhp->getSyncStamp(); 
      // we need to increment the stamp because it is written only once per second or so
      dhp->incrementStamp(itsNpackets);
    } // (!itsSyncMaster)
    else {
      // we are the master, so increase the nextValue and send it to the slaves
      if (firstloop) {
        // set nextstamp equal to actualstamp
        nextstamp.setStamp(seqid, blockid);
        // build in a delay to let the slaves catch up
        nextstamp += itsNPackets * 1500;
        
        // send the startstamp to the slaves
        for (int i = 1; i < itsNRSPOutputs; i++) {
	  ((DH_RSPSync*)datamanager->getOutHolder(i))->setSyncStamp(nextstamp);
          // force a write 
          datamanager->readyWithOutHolder(i);
        }
	firstloop = false;      
      } // end (firstloop) 
      else {
        // increase the syncstamp
        nextstamp += itsNpackets;
        // send the syncstamp to the slaves
        for (int i = 1; i < itsNRSPOutputs; i++) {
	  ((DH_RSPSync*)datamanager().getOutHolder(i))->setSyncStamp(nextstamp);
        }
      }
    } //end (itsIsSyncMaster)


    
    // to do: bufferelements of EPApackets and not ethernet frames !!
    
    if (actualstamp < nextstamp) {
      // old packet received 
      // if exists, overwrite its previous created dummy
      
      // read oldest item in cyclic buffer if buffer is not empty
      if (databuffer->getCount() > 0) {
        
        dataptr = (dataType*)databuffer->getFirstReadPtr(itemid);
        // subtract timestamps to find offset
        offset = actualstamp - dataptr->timestamp;
        // determine itemid to be written
        itemid += offset;
        // release readlock
        databuffer->readyReading();
   
        // overwrite data and invalid flag
        dataptr = (dataType*)databuffer->getUserWritePtr(itemid);
        if (dataptr != 0) {
          memcpy(dataptr->packet, recvframe, framesize);
          dataptr->invalid = 0;
        } 
        // release writelocks
        databuffer->readyWriting();
      
        // read new frame in next loop
        readnew = true;
      }   
    }
    else if (nextstamp + (packetsinframe - 1) < actualstamp) {
      // missed a packet so set invalid flag and missing timestamp
      dataptr = (dataType*)databuffer->getBufferWritePtr(); 
      dataptr->packet[0]  = '\0';
      dataptr->invalid = 1;
      dataptr->timestamp = nextstamp;
     
      // release writelocks
      databuffer->readyWriting();
      
      // read same frame again in next loop
      readnew = false;
    } 
    else {
      // expected packet received so write data into buffer
      dataptr = (dataType*)databuffer->getBufferWritePtr();
      memcpy(dataptr->packet, recvframe, framesize);
      dataptr->invalid = 0;
      dataptr->timestamp = actualstamp;      
     
      // release writelocks
      databuffer->readyWriting();
     
      // read new frame in next loop
      readnew = true;
    } 
  }
}

WH_RSPInput::WH_RSPInput(const string& name, 
                         const ACC::ParameterSet pset,
                         const string device,
                         const string srcMAC,
                         const string destMAC
                         const bool isSyncMaster)
  : WorkHolder ((isSyncMaster ? 1 : 2), 
                1 + (isSyncMaster ? pset.getInt("NoWH_RSP")-1 : 0), 
                name, 
                "WH_RSPInput"),
    itsPset (pset),
    itsDevice(device),
    itsSrcMAC(srcMAC),
    itsDestMAC(destMAC),
    itsSyncMaster(isSyncMaster)
{
  char str[32];
  
  // total amount of RSP-board interfaces
  itsNRSPOutputs = pset.getInt("NoWH_RSP");

  // number of EPA packets per RSP frame
  itsNpackets = pset.getInt("NoPacketsInFrame");
 
  // size of an EPA packet in bytes 
  int sizeofpacket   = ( pset.getInt("polarisations") * 
                          sizeof(complex<int16>) * 
                          pset.getInt("NoRSPBeamlets")
                       ) + 
                       pset.getInt("SzEPAheader"); 
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNpackets * sizeofpacket;

  // create raw ethernet interface to catch incoming RSP data
  itsInputConnection = new TH_Ethernet(itsDevice, itsSrcMAC, itsDestMAC, 0x000 );

  // use  cyclic buffers to hold the rsp data and valid/invalid flag
  itsDataBuffer = new BufferController<dataType>(1000);
 
  // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Sync("DH_Sync"));

  // create outgoing dataholder holding the delay controlled RSP data
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out", pset));

  // create dataholders for RSPInput synchronization
  if (itsIsSyncMaster) {
    // if we are the sync master we need (NoWH_RSP-1) extra outputs
    for (int i = 1; i < itsNRSPOutputs; i++) {
      snprintf(str, 32, "DH_RSPInputSync_out_%d", i);
      getDataManager().addOutDataHolder(i, new DH_RSPSync(str));
    }
  } else {
    // if we are a sync slave we need 1 extra input
    getDataManager().addInDataHolder(1, new DH_RSPSync("DH_RSPSync_in"));
  }
 
}


WH_RSPInput::~WH_RSPInput() 
{
  delete itsDataBuffer;
  delete itsInputConnection;
}


WorkHolder* WH_RSPInput::construct(const string& name,
                                   const ACC::ParameterSet pset,
                                   const string device,
                                   const string srcMAC,
                                   const string destMAC)
{
  return new WH_RSPInput(name, pset, device, srcMAC, destMAC);
}


WH_RSPInput* WH_RSPInput::make(const string& name)
{
  return new WH_RSPInput(name, itsPset, itsDevice, itsSrcMAC, itsDestMAC);
}


void WH_RSPInput::preprocess()
{
  // start up writer thread
  writerinfo.databuffer = itsDataBuffer;
  writerinfo.connection = itsInputConnection;
  writerinfo.framesize = itsSzRSPframe;
  writerinfo.packetsinframe = itsNpackets;
  writerinfo.datamanager = getDataManager();
  writerinfo.syncmaster = itsSyncMaster;
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
  dataType* dataptr;
  timestamp_t syncstamp;
  int seqid, blockid;

  // get delay offset
  inDHp = (DH_Sync*)getDataManager().getInHolder(0);
  inDHp->getNextMainBeat(seqid, blockid);
  syncstamp.setStamp(seqid, blockid);
  
  // get data from cyclic buffer
  dataptr = itsDataBuffer->getBufferReadPtr();

  // write flag to outgoing dataholder
  outDHp->setFlag(dataptr->invalid);

  // write data to outgoing dataholder
  memcpy(outDHp->getBuffer(), dataptr->packet, itsSzRSPframe);
}

void WH_RSPInput::postprocess()
{
  // stop writer thread
  writerinfo.stopthread = true;
}

void WH_RSPInput::dump() 
{
}
