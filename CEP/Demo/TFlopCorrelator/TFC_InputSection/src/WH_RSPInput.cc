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
#include <TFC_InputSection/WH_RSPInput.h>
#include <TFC_Interface/DH_Delay.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_RSPSync.h>
      
#include <Common/hexdump.h>

using namespace LOFAR;


void* WriteToBufferThread(void* arguments)
{
  thread_args* args = (thread_args*)arguments;
  
  char recvframe[9000];
  int seqid, blockid, itemid, offset;
  timestamp_t actualstamp, nextstamp;
  subbandType* subband;
  metadataType* metadata;
  bool readnew = true;
  bool firstloop = true;

  while(1) {
   
    // check stop condition
    if (args->Stopthread == true) {
      pthread_exit(NULL);
    }
   
    // catch a frame from input connection
    if (readnew) {
      args->Connection.recvBlocking( (void*)recvframe, args->FrameSize, 0);
    }

    // get stationid
    *args->StationIDptr =((int*)&recvframe[2])[0]; 
   
    // get the actual timestamp
    seqid   = ((int*)&recvframe[6])[0];
    blockid = ((int*)&recvframe[10])[0];
    actualstamp.setStamp(seqid ,blockid);

    if (!args->Syncmaster) {
    // we are a slave so read the syncstamp
      if (firstloop) {
        // this is the first time
        // we need to force a read, because this inHolder has a lower data rate
        // if the rate difference is 1000, the workholder will read for
        // the first time in the 1000th runstep.
        args->Datamanager->getInHolder(1);
        args->Datamanager->readyWithInHolder(1);
        firstloop = false;
      }      
      DH_RSPSync* dhp = (DH_RSPSync*)args->Datamanager->getInHolder(1);
      nextstamp = dhp->getSyncStamp(); 
      // we need to increment the stamp because it is written only once per second or so
      dhp->incrementStamp(args->nrPacketsInFrame);
    } // (!args->itsSyncMaster)
    else {
      // we are the master, so increase the nextValue and send it to the slaves
      if (firstloop) {
        // set nextstamp equal to actualstamp
        nextstamp.setStamp(seqid, blockid);
        // build in a delay to let the slaves catch up
        nextstamp += args->nrPacketsInFrame * 1500;
        
        // send the startstamp to the slaves
        for (int i = 1; i < args->nrRSPoutputs; i++) {
	  ((DH_RSPSync*)args->Datamanager->getOutHolder(i))->setSyncStamp(nextstamp);
          // force a write 
          args->Datamanager->readyWithOutHolder(i);
        }
	firstloop = false;      
      } // end (firstloop) 
      else {
        // increase the syncstamp
        nextstamp += args->nrPacketsInFrame;
        // send the syncstamp to the slaves
        for (int i = 1; i < args->nrRSPoutputs; i++) {
	  ((DH_RSPSync*)args->Datamanager->getOutHolder(i))->setSyncStamp(nextstamp);
        }
      }
    } //end (itsIsSyncMaster)

    
    if (actualstamp < nextstamp) {
      /* old packet received 
	 Packet can be saved when its dummy is available in cyclic buffer. 
         Otherwise this packet will be lost */
      
      // read oldest item in cyclic buffer if buffer is not empty
      if (args->MetadataBuffer[0]->getBufferCount() > 0) {
        
        metadata = (metadataType*)args->MetadataBuffer[0]->getFirstReadPtr(itemid);
        // subtract timestamps to find offset
        offset = actualstamp - metadata->timestamp;
        // release readlock
        args->MetadataBuffer[0]->readyReading();
   
        if (offset < 0) { 
          // do nothing when packet is too old
          cout << "Received packet is too old and will be lost" << endl;
        }
        else {
          // overwrite its previous created dummy and mark data valid
          itemid += offset;
          int idx;
          for (int p=0; p<args->nrPacketsInFrame; p++) {
            for (int s=0; s<args->nrSubbandsInPacket; s++) {
              subband = (subbandType*)args->SubbandBuffer[s]->getManualWritePtr(itemid);
              idx = p*args->nrPacketsInFrame + s*args->SubbandSize;
              memcpy(subband->data, &recvframe[idx], args->SubbandSize);
              metadata = (metadataType*)args->MetadataBuffer[s]->getManualWritePtr(itemid);
              metadata->invalid = 0;
     
              // release writelocks
              args->SubbandBuffer[s]->readyWriting();
              args->MetadataBuffer[s]->readyWriting();
            }
          }
	}  
      }   
    }
    else if (nextstamp + (args->nrPacketsInFrame - 1) < actualstamp) {
      // missed a packet so create a dummy and mark data invalid
      for (int s=0; s<args->nrSubbandsInPacket; s++) {
        subband = (subbandType*)args->SubbandBuffer[s]->getAutoWritePtr();
        memset(subband->data,0,args->SubbandSize);
        metadata = (metadataType*)args->MetadataBuffer[s]->getAutoWritePtr();
        //metadata->stationid = stationid;
        metadata->invalid = 1;
        metadata->timestamp = nextstamp; 
        
        // release writelocks
        args->SubbandBuffer[s]->readyWriting();
        args->MetadataBuffer[s]->readyWriting();
      }
    
      // read same frame again in next loop
      readnew = false;
    } 
    else {
      /* expected packet received so write data into corresponding subbandbuffer and
	 metadatabuffer */
      int idx;
      for (int p=0; p<args->nrPacketsInFrame; p++) {
        for (int s=0; s<args->nrSubbandsInPacket; s++) {
          subband = (subbandType*)args->SubbandBuffer[s]->getAutoWritePtr();
          idx = p*args->nrPacketsInFrame + s*args->SubbandSize;
          memcpy(subband->data, &recvframe[idx], args->SubbandSize);
          metadata = (metadataType*)args->MetadataBuffer[s]->getAutoWritePtr();
          //metadata->stationid = stationid;
          metadata->invalid = 0;
          metadata->timestamp = actualstamp;      
     
          // release writelocks
          args->SubbandBuffer[s]->readyWriting();
          args->MetadataBuffer[s]->readyWriting();
        }
      }
      // read new frame in next loop
      readnew = true;
    } 
  }
}

WH_RSPInput::WH_RSPInput(const string& name, 
                         ParameterSet& ps,
                         TransportHolder& th,
                         const bool isSyncMaster)
  : WorkHolder ((isSyncMaster ? 1 : 2), 
                ps.getInt32("Input.NSubbands") + (isSyncMaster ? ps.getInt32("Input.NRSP")-1 : 0) , 
                name, 
                "WH_RSPInput"),
    itsTH(th),
    itsPS (ps),
    itsSyncMaster(isSyncMaster)
{
  LOG_TRACE_FLOW_STR("WH_RSPInput constructor");    

  char str[32];

  // get parameters
  itsCyclicBufferSize = ps.getInt32("Input.CyclicBufferSize");
  itsNRSPOutputs = ps.getInt32("Input.NRSP");
  itsNpackets = ps.getInt32("Input.NPacketsInFrame");
  itsNSubbands = ps.getInt32("Input.NSubbands");
  itsNPolarisations = ps.getInt32("Input.NPolarisations");
  itsNSamplesToCopy = ps.getInt32("Input.NSamplesToDH");
 
  // size of an EPA packet in bytes 
  int sizeofpacket   = ( itsNPolarisations * 
                         sizeof(u16complex) * 
                         itsNSubbands) + ps.getInt32("Input.SzEPAheader"); 
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNpackets * sizeofpacket;
 
  // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay",itsNRSPOutputs));

  // create 2 cyclic buffers and a outgoing dataholder per subband.
  itsSubbandBuffer =  new BufferController<subbandType>*[itsNSubbands];
  itsMetadataBuffer = new BufferController<metadataType>*[itsNSubbands];
  for (int s=0; s < itsNSubbands; s++) {
    itsSubbandBuffer[s] =  new BufferController<subbandType>(itsCyclicBufferSize);
    itsMetadataBuffer[s] = new BufferController<metadataType>(itsCyclicBufferSize);  
    snprintf(str, 32, "DH_RSP_out_%d", s);
    getDataManager().addOutDataHolder(s, new DH_RSP(str, itsPS)); 
  }
   
  // create dataholders for RSPInput synchronization
  if (itsSyncMaster) {
    // if we are the sync master we need extra outputs
    for (int i = 0; i < itsNRSPOutputs-1; i++) {
      snprintf(str, 32, "DH_RSPInputSync_out_%d", i);
      getDataManager().addOutDataHolder(itsNSubbands + i, new DH_RSPSync(str));
    }
  } else {
    // if we are a sync slave we need 1 extra input
    getDataManager().addInDataHolder(1, new DH_RSPSync("DH_RSPSync_in"));
  }
 
}


WH_RSPInput::~WH_RSPInput() 
{
  for (int s=0; s < itsNSubbands; s++) {
    delete itsSubbandBuffer[s];
    delete itsMetadataBuffer[s];
  }
  delete [] itsSubbandBuffer;
  delete [] itsMetadataBuffer;
}


WorkHolder* WH_RSPInput::construct(const string& name,
                                   ParameterSet& ps,
                                   TransportHolder& th,
				   const bool isSyncMaster)
{
  return new WH_RSPInput(name, ps, th, isSyncMaster);
}


WH_RSPInput* WH_RSPInput::make(const string& name)
{
  return new WH_RSPInput(name, itsPS, itsTH, itsSyncMaster);
}


void WH_RSPInput::preprocess()
{
  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */
  writerinfo.SubbandBuffer      = itsSubbandBuffer;
  writerinfo.Connection         = itsTH;
  writerinfo.FrameSize          = itsSzRSPframe;
  writerinfo.nrPacketsInFrame   = itsNpackets;
  writerinfo.nrSubbandsInPacket = itsNSubbands;
  writerinfo.nrRSPoutputs       = itsNRSPOutputs;
  writerinfo.SubbandSize        = itsNPolarisations * sizeof(u16complex);
  writerinfo.Datamanager        = &getDataManager();
  writerinfo.Syncmaster         = itsSyncMaster;
  writerinfo.Stopthread         = false;
  writerinfo.StationIDptr       = &itsStationID;
  
  if (pthread_create(&writerthread, NULL, WriteToBufferThread, &writerinfo) < 0)
  {
    perror("WH_RSPInput: thread creation failure");
    exit(1);
  }
}

void WH_RSPInput::process() 
{ 
  DH_RSP* rspDHp;
  metadataType* metadata;
  
  // get delay 
  DH_Delay* delayDHp = (DH_Delay*)getDataManager().getInHolder(0);
  ASSERTSTR(((itsStationID < 0) && (itsStationID > 2)), "WH_RSPInput: Invalid station ID");
  int delay = delayDHp->getDelay(itsStationID); 

  for (int s=0; s < itsNSubbands; s++) {

    // get outgoing dataholder
    rspDHp = (DH_RSP*)getDataManager().getOutHolder(s);

    // copy 'itsNSamplesToCopy' subband samples from buffer to outgoing dataholder
    memcpy(rspDHp->getBuffer(), 
           itsSubbandBuffer[s]->getBlockReadPtr(delay, itsNSamplesToCopy),
           itsNSamplesToCopy * itsNPolarisations * sizeof(u16complex));
    
    // copy metadata from cyclic buffer to outgoing dataholder
    int count = 0;
    for (int t=0; t<itsNSamplesToCopy; t++) {
      metadata = itsMetadataBuffer[s]->getBlockReadPtr(delay, 1);
      if (metadata->invalid > 0) {
	count++;
      }
      if (t==0) {
        rspDHp->setStationID(itsStationID);        // station involved
        rspDHp->setTimeStamp(metadata->timestamp); // timestamp of first subband sample
      }
    }
    rspDHp->setInvalidCount(count);                // number invalid subband samples
  } 
  
  // dump the output (for debugging)
  cout << "WH_RSPInput output : " << endl;
  hexdump(rspDHp->getBuffer(), rspDHp->getBufferSize() * sizeof(DH_RSP::BufferType));
  
}

void WH_RSPInput::postprocess()
{
  // stop writer thread
  writerinfo.Stopthread = true;
}

void WH_RSPInput::dump() const 
{
}
