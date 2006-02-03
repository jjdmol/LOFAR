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

// Application specific includes
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_Delay.h>
#include <TFC_Interface/DH_RSPSync.h>
#include <TFC_InputSection/WH_RSPInput.h>

#include <Common/hexdump.h>

using namespace LOFAR;

WH_RSPInput::WH_RSPInput(const string& name, 
                         ParameterSet& ps,
                         TransportHolder& th,
                         const bool isSyncMaster)
  : WorkHolder ((isSyncMaster ? 1 : 2), 
                ps.getInt32("Data.NSubbands") + (isSyncMaster ? ps.getInt32("Data.NStations")-1 : 0) , 
                name, 
                "WH_RSPInput"),
    itsTH(th),
    itsPS (ps),
    itsSyncMaster(isSyncMaster),
    itsFirstProcessLoop(true),
    itsBBuffer(0)
{
  LOG_TRACE_FLOW_STR("WH_RSPInput constructor");    

  char str[32];

  // get parameters
  itsCyclicBufferSize = ps.getInt32("Input.NSamplesToBuffer");
  itsNRSPOutputs = ps.getInt32("Data.NStations");
  itsNSubbands = ps.getInt32("Data.NSubbands");
  itsNPolarisations = ps.getInt32("Data.NPolarisations");
  itsNSamplesToCopy = ps.getInt32("Data.NSamplesToIntegrate");
 
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
  delete itsBBuffer;
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
  
  writerinfo.BBuffer            = itsBBuffer;
  writerinfo.Connection         = &itsTH;
  writerinfo.PayloadSize        = itsPS.getInt32("Input.PayloadSize");
  writerinfo.IPHeaderSize       = itsPS.getInt32("Input.IPHeaderSize");
  writerinfo.nrPacketsInFrame   = itsPS.getInt32("Input.NPacketsInFrame");
  writerinfo.nrSubbandsInPacket = itsNSubbands;
  writerinfo.nrRSPoutputs       = itsNRSPOutputs;
  writerinfo.SubbandSize        = itsNPolarisations * sizeof(u16complex);
  writerinfo.Stopthread         = false;
  writerinfo.StationIDptr       = &itsStationID;
  writerinfo.EPAHeaderSize      = itsPS.getInt32("Input.SzEPAheader");
  writerinfo.EPAPacketSize      = itsPS.getInt32("Input.SzEPApayload");
  writerinfo.IsMaster           = itsSyncMaster;
  
  if ((itsTH.getType() == "TH_File") || (itsTH.getType() == "TH_Null")) {
    // if we are reading from file, overwriting the buffer should not be allowed
    // this way we can work with smaller files
    itsBBuffer->setAllowOverwrite(false);
  }

  if (pthread_create(&writerthread, NULL, ReaderThread, &writerinfo) < 0)
  {
    perror("WH_RSPInput: thread creation failure");
    exit(1);
  }
}

void WH_RSPInput::preprocess()
{
  // create the buffer controller.
  itsBBuffer = new BeamletBuffer(itsCyclicBufferSize, itsNSubbands, itsCyclicBufferSize/6, itsCyclicBufferSize/6);
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
  cout<<"end of WH_RSPInput::preprocess"<<endl;cout.flush();
}

void WH_RSPInput::process() 
{ 
  cout<<"begin of WH_RSPInput::process"<<endl;cout.flush();
  itsProcessTimer->start();
  DH_RSP* rspDHp;
  DH_Delay* delayDHp;
  timestamp_t delayedstamp;


  if (itsSyncMaster) {

    if (itsFirstProcessLoop) {
      // let the buffer fill
      sleep(1);

      // start the buffer and get the stamp at which we need to start reading
      cout<<"master asking buffer for good place to start"<<endl;cout.flush();
      itsSyncedStamp = itsBBuffer->startBufferRead();
      cout<<"SyncedStamp on master: "<<itsSyncedStamp<<endl; cout.flush();
      
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
      cout<<"slave reading from master"<<endl;cout.flush();
      DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
      getDataManager().readyWithInHolder(1);
      dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
      itsSyncedStamp = dhp->getSyncStamp(); 
      itsDelayTimer->stop();
      cout<<"SyncedStamp on slave: "<<itsSyncedStamp<<endl; cout.flush();
      
      // start the buffer and set the stamp at which we need to start reading
      itsBBuffer->startBufferRead(itsSyncedStamp);
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
  itsBBuffer->getElements(subbandbuffer,
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
    if(itsSyncMaster) {
      cout << "WH_RSPInput output (stamp: "<<delayedstamp<<"): " << endl;
      hexdump(rspDHp->getBuffer(), 200 * sizeof(DH_RSP::BufferType)); 
    }
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
  itsBBuffer->clear();
  cout<<"buffer cleared"<<endl;
  ASSERTSTR(pthread_join(writerthread, NULL) == 0, "thread could not be joined");
  //  pthread_cancel(writerthread);
  sleep(2);
}

void WH_RSPInput::dump() const 
{
}
