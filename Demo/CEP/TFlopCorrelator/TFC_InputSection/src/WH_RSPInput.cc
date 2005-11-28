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
#include <Transport/TransportHolder.h>
#include <Transport/TH_File.h>

// Application specific includes
#include <TFC_InputSection/WH_RSPInput.h>
#include <TFC_Interface/DH_Delay.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_RSPSync.h>

#include <Common/hexdump.h>

using namespace LOFAR;

void printTimers(vector<NSTimer*>& timers)
{
  vector<NSTimer*>::iterator it = timers.begin();
  for (; it != timers.end(); it++) {
    (*it)->print(cout);
  }

}
void cleanupWriteBuffer(void* args)
{
  cout<<"\nThread Timers:"<<endl;
  printTimers(*((vector<NSTimer*>*)args));
}

void* WriteToBufferThread(void* arguments)
{
  LOG_TRACE_FLOW_STR("WH_RSPInput WriterThread");   

  thread_args* args = (thread_args*)arguments;
  cout<<"Framesize: "<<args->FrameSize<<endl;
  int seqid, blockid;
  timestamp_t actualstamp, expectedstamp;
  bool readnew = true;
  bool firstloop = true;

  // buffer for incoming rsp data
  char recvframe[args->FrameSize];

  // define a block of dummy subband data
  SubbandType dummyblock[args->nrPacketsInFrame];
  memset(dummyblock, 0, args->nrPacketsInFrame*sizeof(SubbandType));

  // used for debugging
  int cnt_missed = 0;
  int cnt_rewritten = 0;
  int cnt_total = 0;

  int speedCounter = 0;

  vector<NSTimer*> itsTimers;
  NSTimer threadTimer("threadTimer");
  NSTimer receiveTimer("receiveTimer");
  NSTimer writeTimer("writeTimer");
  NSTimer rewriteTimer("rewriteTimer");
  NSTimer writeDummyTimer("writeDummyTimer");
  itsTimers.push_back(&threadTimer);
  itsTimers.push_back(&receiveTimer);
  itsTimers.push_back(&writeTimer);
  itsTimers.push_back(&rewriteTimer);
  itsTimers.push_back(&writeDummyTimer);

  //  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  //  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  //pthread_cleanup_push(&cleanupWriteBuffer, &itsTimers);

  // init Transportholder
  ASSERTSTR(args->Connection->init(), "Could not init TransportHolder");

  int strideSize = args->EPAPacketSize / sizeof(SubbandType);

  while(!args->Stopthread) {
    threadTimer.start();

    // check stop condition
    if (args->Stopthread == true) {
      pthread_exit(NULL);
    }
    // catch a frame from input connection
    if (readnew) {
      try {
	if (args->Connection != 0){
	  receiveTimer.start();
	  args->Connection->recvBlocking( (void*)recvframe, args->FrameSize, 0);
	  receiveTimer.stop();
	}
      } catch (Exception& e) {
	LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder, stopping thread");
	pthread_exit(NULL);
      }	
    }
  

    if (args->Connection->getType() != "TH_Null") {
      // get the actual timestamp of first EPApacket in frame
      seqid   = ((int*)&recvframe[8])[0];
      blockid = ((int*)&recvframe[12])[0];
      actualstamp.setStamp(seqid ,blockid);
    }
  
    // firstloop
    if (firstloop) {
      expectedstamp.setStamp(seqid, blockid); // init expectedstamp
      
      //get stationid
      //*args->StationIDptr =((int*)&recvframe[4])[0];
      if (args->IsMaster) {  // temporary hardcoded statioID's master->0, slave->1 
        *args->StationIDptr = 0;  
      }
      else {
        *args->StationIDptr = 1;  
      } //end (temporary hardcoded statioID's)
      firstloop = false;
    }
    
    // check and process the incoming data
    if (actualstamp < expectedstamp) {
      rewriteTimer.start();
      /* old packet received 
	 Packet can be saved when its dummy is available in cyclic buffer. 
         Otherwise this packet will be lost */

      int idx;
      for (int p=0; p<args->nrPacketsInFrame; p++) {
        idx = (p*args->EPAPacketSize) + args->EPAHeaderSize;
        cnt_rewritten++;
        if (!args->BufControl->rewriteElements((SubbandType*)&recvframe[idx], actualstamp)) {
          cnt_rewritten--;
          break;  
	}
        actualstamp++;   
      }
      //cout << cnt_rewritten << " delayed packets recovered." << endl;  // debugging

      // read new frame in next loop
      readnew = true;
      // do not increase the expectedstamp
      rewriteTimer.stop();
    } else if (actualstamp > expectedstamp) {
      writeDummyTimer.start();
      // missed a packet so create dummy for that missing packet
      args->BufControl->writeDummy((SubbandType*)dummyblock, expectedstamp, args->nrPacketsInFrame);
      cnt_missed += args->nrPacketsInFrame;
      //cout << "Dummy created for " << cnt_missed << " missed packets." << endl; // debugging
      // read same frame again in next loop
      readnew = false;
      // increase the expectedstamp
      expectedstamp += args->nrPacketsInFrame; 
      writeDummyTimer.stop();
    } else {
      writeTimer.start();
      // expected packet received so write data into corresponding buffer
      int idx;
#if 1
      args->BufControl->writeElements((SubbandType*)&recvframe[args->EPAHeaderSize], actualstamp, args->nrPacketsInFrame, strideSize);
      actualstamp += args->nrPacketsInFrame;
#else
      for (int p=0; p<args->nrPacketsInFrame; p++) {
	idx = (p*args->EPAPacketSize) + args->EPAHeaderSize;
	args->BufControl->writeElements((SubbandType*)&recvframe[idx], actualstamp);
      	actualstamp++;
      }
#endif
      // read new frame in next loop
      readnew = true;
      // increase the expectedstamp
      expectedstamp += args->nrPacketsInFrame; 
      writeTimer.stop();
    }
#if 0
    cnt_total++;
    if (speedCounter-- < 1) {
      threadTimer.stop();
      if (args->IsMaster) {
	cout<<"master: ";
      } else {
	cout<<"slave : ";
      }
      cout << cnt_total*args->nrPacketsInFrame <<" EPApackets: "<< cnt_missed<<" missed and "<<cnt_rewritten<<" rewritten"<<endl;
      cnt_total = cnt_missed = cnt_rewritten = 0;
      speedCounter = 999;

      printTimers (itsTimers);
      threadTimer.start();
    }
#endif
    threadTimer.stop();
  }
  //pthread_cleanup_pop(1);
  printTimers(itsTimers);
  pthread_exit(NULL);
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
    itsFirstProcessLoop(true),
    itsBufControl(0)
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
  itsEPAPacketSize = ps.getInt32("Input.SzEPApayload") + itsEPAHeaderSize;
 
  // size of a RSP frame in bytes
  itsSzRSPframe = itsNPackets * itsEPAPacketSize;
 
  // create incoming dataholder holding the delay information 
  getDataManager().addInDataHolder(0, new DH_Delay("DH_Delay",itsNRSPOutputs));
  //  getDataManager().setAutoTriggerIn(0, false);
 
  // create a outgoing dataholder for each subband
  for (int s=0; s < itsNSubbands; s++) {
    snprintf(str, 32, "DH_RSP_out_%d", s);
    getDataManager().addOutDataHolder(s, new DH_RSP(str, itsPS)); 
  }
   
  // create dataholders for RSPInput synchronization
  if (itsSyncMaster) {
    // if we are the sync master we need extra outputs
    for (int i = 0; i < itsNRSPOutputs-1; i++) {
      snprintf(str, 32, "DH_RSPInputSync_out_%d", i);
      getDataManager().addOutDataHolder(itsNSubbands + i, new DH_RSPSync(str));
      //don't use autotriggering when sending synced stamps to slaves
      getDataManager().setAutoTriggerOut(itsNSubbands + i, false);
    }
  } else {
    // if we are a sync slave we need 1 extra input
    getDataManager().addInDataHolder(1, new DH_RSPSync("DH_RSPSync_in"));
    getDataManager().setAutoTriggerIn(1, false);
  }
}


WH_RSPInput::~WH_RSPInput() 
{
  delete itsBufControl;
  // itsTH is deleted by the AH because it is created there
  // delete &itsTH;
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


void WH_RSPInput::startThread()
{
  
  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */
  LOG_TRACE_FLOW_STR("WH_RSPInput starting thread");   
  
  writerinfo.BufControl         = itsBufControl;
  writerinfo.Connection         = &itsTH;
  writerinfo.FrameSize          = itsSzRSPframe;
  writerinfo.nrPacketsInFrame   = itsNPackets;
  writerinfo.nrSubbandsInPacket = itsNSubbands;
  writerinfo.nrRSPoutputs       = itsNRSPOutputs;
  writerinfo.SubbandSize        = itsNPolarisations * sizeof(u16complex);
  writerinfo.Stopthread         = false;
  writerinfo.StationIDptr       = &itsStationID;
  writerinfo.EPAHeaderSize      = itsEPAHeaderSize;
  writerinfo.EPAPacketSize      = itsEPAPacketSize;
  writerinfo.IsMaster           = itsSyncMaster;
  
  if ((dynamic_cast<TH_File*> (&itsTH)) != 0) {
    // if we are reading from file, overwriting the buffer should not be allowed
    // this way we can work with smaller files
    itsBufControl->setAllowOverwrite(false);
  }

  if (pthread_create(&writerthread, NULL, WriteToBufferThread, &writerinfo) < 0)
  {
    perror("WH_RSPInput: thread creation failure");
    exit(1);
  }
}

void WH_RSPInput::preprocess()
{
  // create the buffer controller.
  itsBufControl = new BufferController(itsCyclicBufferSize, itsNSubbands);
  startThread();
  itsPrePostTimer = new NSTimer("pre/post");
  itsProcessTimer = new NSTimer("process");
  itsDelayTimer = new NSTimer("delay");
  itsGetElemTimer = new NSTimer("getElem");
  itsTimers.push_back(itsPrePostTimer);
  itsTimers.push_back(itsProcessTimer);
  itsTimers.push_back(itsDelayTimer);
  itsTimers.push_back(itsGetElemTimer);
  itsPrePostTimer->start();
}

void WH_RSPInput::process() 
{ 
  itsProcessTimer->start();
  DH_RSP* rspDHp;
  DH_Delay* delayDHp;
  timestamp_t delayedstamp;


  if (itsSyncMaster) {

    if (itsFirstProcessLoop) {
      // let the buffer fill
      sleep(1);

      // start the buffer and get the stamp at which we need to start reading
      itsSyncedStamp = itsBufControl->startBufferRead();
      cout<<"SyncedStamp on master: "<<itsSyncedStamp<<endl;
      
      // we are the master, so send the syncstamp to the slaves
      // send the syncstamp to the slaves
      itsDelayTimer->start();
      for (int i = 1; i < itsNRSPOutputs; i++) {
	((DH_RSPSync*)getDataManager().getOutHolder(itsNSubbands + i - 1))->setSyncStamp(itsSyncedStamp);
	// force the write because autotriggering is off 
	getDataManager().readyWithOutHolder(itsNSubbands + i - 1);
      }
      itsFirstProcessLoop = false;
      itsDelayTimer->stop();

    } else { // not the first loop
      // increase the syncstamp
      itsSyncedStamp += itsNSamplesToCopy;
    }

  } else {  // sync slave

    if (itsFirstProcessLoop) {
      // we are a slave so read the syncstamp
      itsDelayTimer->start();
      DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
      getDataManager().readyWithInHolder(1);
      dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
      itsSyncedStamp = dhp->getSyncStamp(); 
      itsDelayTimer->stop();
      cout<<"SyncedStamp on slave: "<<itsSyncedStamp<<endl;
      
      // start the buffer and set the stamp at which we need to start reading
      itsBufControl->startBufferRead(itsSyncedStamp);
      itsFirstProcessLoop = false; 

    } else {  //not the first loop
      // increase the syncstamp
      itsSyncedStamp += itsNSamplesToCopy;
    }  
  }

  // delay control
  delayDHp = (DH_Delay*)getDataManager().getInHolder(0);
  // Get delay from the delay controller
  delayedstamp = itsSyncedStamp + delayDHp->getDelay(itsStationID);    
  //delayedstamp = itsSyncedStamp;


  /* startstamp is the synced and delay-controlled timestamp to 
     start from in cyclic buffer */
  int invalidcount;
  vector<SubbandType*> subbandbuffer;
  for (int s=0; s < itsNSubbands; s++) {
    rspDHp = (DH_RSP*)getDataManager().getOutHolder(s);
    subbandbuffer.push_back((SubbandType*)rspDHp->getBuffer());
  }
  // get the data from the cyclic buffer
  itsGetElemTimer->start();
  itsBufControl->getElements(subbandbuffer,
                             invalidcount, 
                             delayedstamp, 
                             itsNSamplesToCopy);
  itsGetElemTimer->stop();

  // fill in the outgoing dataholders
  for (int s=0; s < itsNSubbands; s++) { 
    rspDHp = (DH_RSP*)getDataManager().getOutHolder(s);
    
    // fill in the data
    rspDHp->setStationID(itsStationID);
    rspDHp->setInvalidCount(invalidcount);
    rspDHp->setTimeStamp(delayedstamp);   
    rspDHp->setDelay(delayDHp->getDelay(itsStationID));

#if 0
    // dump the output (for debugging)
    cout << "WH_RSPInput output (stamp: "<<delayedstamp<<"): " << endl;
    hexdump(rspDHp->getBuffer(), rspDHp->getBufferSize() * sizeof(DH_RSP::BufferType)); 
#endif
  }    

#if 0
  if(itsSyncMaster) {
    cout<<"master has stamp: "<<delayedstamp<<endl;
  } else {
    cout<<"slave has stamp: "<<delayedstamp<<endl;
  }
#endif
  itsProcessTimer->stop();
}

void WH_RSPInput::postprocess()
{
  sleep(1);
  itsPrePostTimer->stop();

  cout<<"\nWH_Timers:"<<endl;
  printTimers(itsTimers);
  delete itsPrePostTimer;
  delete itsProcessTimer;
  delete itsDelayTimer;
  delete itsGetElemTimer;
  itsPrePostTimer = 0;
  itsProcessTimer = 0;
  itsDelayTimer = 0;
  itsGetElemTimer = 0;


  cout<<"in WH_RSPInput postprocess"<<endl;
  //writerinfo.Connection         = 0;
  // stop writer thread
  writerinfo.Stopthread         = true;
  pthread_join(writerthread, NULL);
  itsBufControl->clear();
  //  pthread_cancel(writerthread);
  sleep(2);
}

void WH_RSPInput::dump() const 
{
}


