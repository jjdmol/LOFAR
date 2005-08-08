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
  LOG_TRACE_FLOW_STR("WH_RSPInput WriterThread");   
   
  thread_args* args = (thread_args*)arguments;
  
  char recvframe[9000];
  int seqid, blockid;
  timestamp_t actualstamp, nextstamp;
  bool readnew = true;
  bool firstloop = true;

  char dummyblock[args->nrPacketsInFrame];
  memset(dummyblock,0,args->nrPacketsInFrame);

  while(1) {
   
    // check stop condition
    if (args->Stopthread == true) {
      pthread_exit(NULL);
    }
   
    // catch a frame from input connection
    if (readnew) {
      args->Connection.recvBlocking( (void*)recvframe, args->FrameSize, 0);
    }
   
    // get the actual timestamp of first EPApacket in frame
    seqid   = ((int*)&recvframe[6])[0];
    blockid = ((int*)&recvframe[10])[0];
    actualstamp.setStamp(seqid ,blockid);


    // firstloop
    if (firstloop) {
      nextstamp.setStamp(seqid, blockid);            // init nextstamp
      *args->StationIDptr =((int*)&recvframe[2])[0]; // get stationid
      firstloop = false;
    }

    // check and process the incoming data
    if (actualstamp < nextstamp) {
      /* old packet received 
	 Packet can be saved when its dummy is available in cyclic buffer. 
         Otherwise this packet will be lost */
     
      // overwrite its previous created dummy
      int idx;  //To do: rectmatrix?? 
      for (int p=0; p<args->nrPacketsInFrame; p++) {
        for (int s=0; s<args->nrSubbandsInPacket; s++) {
          idx = (p*args->EPAPacketSize) + args->EPAHeaderSize + (s*args->SubbandSize);
          args->BufControl[s]->rewriteElements(&recvframe[idx], actualstamp, 1);
        }
        actualstamp++;
      }
      // read new frame in next loop
      readnew = true;
    }
    else if (nextstamp + (args->nrPacketsInFrame - 1) < actualstamp) {
      // missed a packet so create dummy
      for (int s=0; s<args->nrSubbandsInPacket; s++) {
        args->BufControl[s]->writeElements(dummyblock, actualstamp,args-> nrPacketsInFrame, 1);
      }
      // read same frame again in next loop
      readnew = false;
    } 
    else {
      // expected packet received so write data into corresponding buffer
      int idx;
      for (int p=0; p<args->nrPacketsInFrame; p++) {
        for (int s=0; s<args->nrSubbandsInPacket; s++) {
          idx = (p*args->EPAPacketSize) + args->EPAHeaderSize + (s*args->SubbandSize);
          args->BufControl[s]->writeElements(&recvframe[idx], actualstamp, 1, 0);
        }
      }
      // read new frame in next loop
      readnew = true;
    }
    // increase the nextstamp
    nextstamp += args->nrPacketsInFrame; 
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
    itsSyncMaster(isSyncMaster),
    itsFirstProcessLoop(true)
{
  LOG_TRACE_FLOW_STR("WH_RSPInput constructor");    

  char str[32];

  // get parameters
  itsCyclicBufferSize = ps.getInt32("Input.CyclicBufferSize");
  itsNRSPOutputs = ps.getInt32("Input.NRSP");
  itsNPackets = ps.getInt32("Input.NPacketsInFrame");
  itsNSubbands = ps.getInt32("Input.NSubbands");
  itsNPolarisations = ps.getInt32("Input.NPolarisations");
  itsNSamplesToCopy = ps.getInt32("Input.NSamplesToDH");
  itsEPAHeaderSize =  ps.getInt32("Input.SzEPAheader");
  
 
  // size of an EPA packet in bytes 
  itsEPAPacketSize = ( itsNPolarisations * 
                       sizeof(u16complex) * 
                      itsNSubbands) + itsEPAHeaderSize; 
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNPackets * itsEPAPacketSize;
 
  // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay",itsNRSPOutputs));

  // create a buffer controller and outgoing dataholder per subband.
  itsBufControl = new BufferController*[itsNSubbands];
  for (int s=0; s < itsNSubbands; s++) {
    itsBufControl[s] = new BufferController(itsPS);
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
    delete itsBufControl[s];
  }
  delete [] itsBufControl;
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
  LOG_TRACE_FLOW_STR("WH_RSPInput preprocess");   
  
  writerinfo.BufControl         = itsBufControl;
  writerinfo.Connection         = itsTH;
  writerinfo.FrameSize          = itsSzRSPframe;
  writerinfo.nrPacketsInFrame   = itsNPackets;
  writerinfo.nrSubbandsInPacket = itsNSubbands;
  writerinfo.nrRSPoutputs       = itsNRSPOutputs;
  writerinfo.SubbandSize        = itsNPolarisations * sizeof(u16complex);
  writerinfo.Stopthread         = false;
  writerinfo.StationIDptr       = &itsStationID;
  writerinfo.EPAHeaderSize      = itsEPAHeaderSize;
  writerinfo.EPAPacketSize      = itsEPAPacketSize;
  
  if (pthread_create(&writerthread, NULL, WriteToBufferThread, &writerinfo) < 0)
  {
    perror("WH_RSPInput: thread creation failure");
    exit(1);
  }
}

void WH_RSPInput::process() 
{ 
  DH_RSP* rspDHp;
  DH_Delay* delayDHp;
  timestamp_t delayedstamp;

  // get delay from delaycontroller 
  delayDHp = (DH_Delay*)getDataManager().getInHolder(0);
  
  if (!itsSyncMaster) {
    // we are a slave so read the syncstamp
    if (itsFirstProcessLoop) {
      // this is the first time
      // we need to force a read, because this inHolder has a lower data rate
      // if the rate difference is 1000, the workholder will read for
      // the first time in the 1000th runstep.
      getDataManager().getInHolder(1);
      getDataManager().readyWithInHolder(1);
      itsFirstProcessLoop = false;;
    }      
    DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
    itsSyncedStamp = dhp->getSyncStamp(); 
    
    // we need to increment the stamp because it is written only once per second or so
    dhp->incrementStamp(itsNPackets);
  } // (!args->itsSyncMaster)
  else {
    // we are the master, so increase the nextValue and send it to the slaves
    if (itsFirstProcessLoop) {
      // set startstamp equal to first stamp in cyclicbuffer
      itsSyncedStamp = itsBufControl[0]->getFirstStamp();
      // build in a delay to let the slaves catch up
      itsSyncedStamp += itsNPackets * 1500;
        
      // send the synced startstamp to the slaves
      for (int i = 1; i < itsNRSPOutputs; i++) {
	((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsSyncedStamp);
        // force a write 
        getDataManager().readyWithOutHolder(i);
      }
      itsFirstProcessLoop = false;      
    }
    else {
      // increase the syncstamp
      itsSyncedStamp += itsNPackets;
      // send the syncstamp to the slaves
      for (int i = 1; i < itsNRSPOutputs; i++) {
        ((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsSyncedStamp);
      }
    }
  } //end (itsIsSyncMaster)

  // delay control
  delayDHp = (DH_Delay*)getDataManager().getInHolder(0);
  delayedstamp = itsSyncedStamp + delayDHp->getDelay(itsStationID);    


  /* startstamp is the synced and delay-controlled timestamp to 
     start from in cyclic buffer */
  int invalidcount;
  for (int s=0; s < itsNSubbands; s++) {

    // get outgoing dataholder
    rspDHp = (DH_RSP*)getDataManager().getOutHolder(s);

    // copy 'itsNSamplesToCopy' subband samples from buffer into outgoing dataholder
    itsBufControl[s]->getElements(rspDHp->getBuffer(),
                                  invalidcount, 
                                  delayedstamp, 
                                  itsNSamplesToCopy);

    rspDHp->setStationID(itsStationID);
    rspDHp->setInvalidCount(invalidcount);  // number of invalid subbands
    rspDHp->setTimeStamp(delayedstamp);   
    rspDHp->setDelay(delayDHp->getDelay(itsStationID));
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


