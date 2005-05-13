//#  WH_RSPInput.cc: Catch RSP ethernet frames and synchronize RSP inputs
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
#include <tinyCEP/Profiler.h>

// Application specific includes
#include <WH_RSPInput.h>
#include <DH_RSP.h>
#include <DH_RSPSync.h>
#include <BufferingController.h>

using namespace LOFAR;

ProfilingState WH_RSPInput::theirOldDataState;
ProfilingState WH_RSPInput::theirMissingDataState;

WH_RSPInput::WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
	                 const bool isSyncMaster) 
  : WorkHolder (1,
                //if SyncMaster we have 'NoWH_RSP' extra outputs
		1 + (isSyncMaster?kvm.getInt("NoWH_RSP", 2) : 0),
		name, 
		"WH_RSPInput"),
    itsKVM (kvm),
    itsIsSyncMaster(isSyncMaster),
    itsNextStamp(-1,0),
    itsReadNext(true)
{
  char str[32];
  
  // get parameters
  itsNRSPOutputs   = kvm.getInt("NoWH_RSP", 2);  
  itsPolarisations = kvm.getInt("polarisations",2);           
  itsNbeamlets     = kvm.getInt("NoRSPBeamlets", 92);   // number of EPA-packet beamlets per OutDataholder
  itsNpackets      = kvm.getInt("NoPacketsInFrame", 8); // number of EPA-packets in RSP-ethernetframe
  itsSzEPAheader   = kvm.getInt("SzEPAheader", 14);     // headersize in bytes
  itsSzEPApacket   = (itsPolarisations * sizeof(complex<int16>) * kvm.getInt("NoRSPBeamlets", 92)) + itsSzEPAheader; // packetsize in bytes

  // create incoming rsp-dataholder   
  getDataManager().addInDataHolder(0, new DH_RSP("DH_RSP_in", itsKVM));
  
  // create outgoing rsp-dataholder
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out", itsKVM));

  //**** delete SyncMaster code when the DelayController which controls the delay and synchronization is build ****
  if (itsIsSyncMaster) {
    // We are the SyncMaster so we need extra outputs (NoWH_RSP)
    for (int i = 1; i <= itsNRSPOutputs; i++) {
      snprintf(str, 32, "DH_Sync_out_%d", i);
      getDataManager().addOutDataHolder(i, new DH_RSPSync(str));
    }
  }
  
  // create a cyclic buffer with 100 'DH_RSP_out' elements
  itsBufControl = new BufferingController(0,getDataManager().getOutHolder(0), 100);

  //**************************************************************************************************************

  // use cyclic buffer on output channel 0
  //((DataManager)getDataManager()).setOutBufferingProperties(0, false);
   
  // do not use autotriggering on the input
  // 'new read' trigger will be set manually
  getDataManager().setAutoTriggerIn(0, false); 

  // init profiling states
  theirOldDataState.init ("WH_RSPInput old packets", "yellow");
  theirMissingDataState.init ("WH_RSPInput missing packets", "orange");  
}

WH_RSPInput::~WH_RSPInput() 
{
  delete itsBufControl;
}

WorkHolder* WH_RSPInput::construct(const string& name,
                              const KeyValueMap kvm,
			      const bool isSyncMaster) 
{
  return new WH_RSPInput(name, kvm, isSyncMaster);
}

WH_RSPInput* WH_RSPInput::make(const string& name)
{
  return new WH_RSPInput(name, itsKVM, itsIsSyncMaster);
}

void WH_RSPInput::preprocess()
{
  itsBufControl->preprocess();
}

void WH_RSPInput::process() 
{ 
  if (itsReadNext) {
    // read next packet from input
    getDataManager().getInHolder(0);
    getDataManager().readyWithInHolder(0);
  }

  //**** delete SyncMaster code when the DelayController which controls the delay and synchronization is build ****
  int startDelay;  
  
  if (itsIsSyncMaster) {
    // we are the SyncMaster, so increase the nextValue and send it to the slaves
    if (itsNextStamp.getSeqId() == -1) {
      // this is the first loop
      // so take the current timestamp and determine the time stamp at which we all will start
      DH_RSP* inDHp = (DH_RSP*)getDataManager().getInHolder(0);
      itsNextStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());
  
      if (!itsKVM.getBool("useRealRSPBoard", false)) {
	// we are not using the real rspboards, so no delay
	startDelay = 0;;
      } else {
	// we are using the real rsp boards, put in a delay to
	// let the slaves catch up
	startDelay = itsNpackets * 1500;
      }
      for (int i = 1; i <= itsNRSPOutputs; i++) {
	((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsNextStamp + startDelay);
        // we need to force a write, because this outHolder has a lower data rate
        // if the rate is 1000, the workholder will write for
        // the first time in the 1000th runstep.
        getDataManager().readyWithOutHolder(i);
      }      
    }// 
    else {
      itsNextStamp += itsNpackets;
      for (int i = 1; i < itsNRSPOutputs; i++) {
	((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsNextStamp);
      }
    }
  }
  //****************************************************************************************************************
  else {
    // we are not SyncMaster
    if (itsNextStamp.getSeqId() == -1) {
      // this is the first loop
      // so take the current timestamp
      DH_RSP* inDHp = (DH_RSP*)getDataManager().getInHolder(0);
      itsNextStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());
    }
    else {
      itsNextStamp += itsNpackets;
    }      
  }

  DH_RSP* inDHp;
  DH_RSP* outDHp;
  bool newStamp = false;
  SyncStamp thisStamp;
  int amountToCopy =  itsNbeamlets * itsPolarisations * sizeof(complex<int16>);

  while (!newStamp) {
    
    // get incoming RSP data
    inDHp = (DH_RSP*)getDataManager().getInHolder(0);
    thisStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());

    if (thisStamp < itsNextStamp) { 
      // this packets time stamp is too old
      // packet could be delayed in ethernet connection link
  
      // to do: store this packet in right place of output buffer to 
      // overwrite already made dummy-data for this timestamp
      
      // set profiling state
      theirMissingDataState.leave();
      theirOldDataState.enter();

      LOG_TRACE_COND_STR("Old packet received");
      
      // stay in the while loop and trigger a new read
      getDataManager().readyWithInHolder(0);
      newStamp = false;
    } 
    else if (itsNextStamp + (itsNpackets - 1) < thisStamp) {
      // we missed a packet 

      // set profiling state
      theirOldDataState.leave();
      theirMissingDataState.enter();
      
      // copy dummy data for missing timestamp in OutDataholder 
      outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
      outDHp->setFlag( 1 ); // mark data as invalid
      memset( outDHp->getBuffer(), 0, amountToCopy);
      
      // step out of the while loop and do not trigger a new read 
      // so this packet will be read again next loop
      newStamp = true;
      itsReadNext = false;
    }
    else { 
      // excpected packet received

      // reset profiling states
      theirMissingDataState.leave();
      theirOldDataState.leave();

      // copy contents of InDataHolder to OutDataholder
      outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
      outDHp->setFlag( 0 ); // mark data as valid
      memcpy(outDHp->getBuffer(), inDHp->getBuffer(), amountToCopy);
      
      // step out of the while loop and trigger a new read
      newStamp = true;
      itsReadNext = true;
    }
  } 
}

void WH_RSPInput::dump() {
  cout<<"DUMP OF WH_RSPInput: "<<getName()<<endl;
  DH_RSP* inDHp;
  DH_RSP* outDHp;
  inDHp = (DH_RSP*)getDataManager().getInHolder(0);
  outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
  
    
  // determine blocksizes of char-based InHolder and complex<int16-based> OutHolder
  int char_blocksize   = itsNbeamlets * itsPolarisations * sizeof(complex<int16>);
      
  for (int i=0;i<itsNpackets;i++) {
      outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
      cout<<"packet: "<<"   ";
      for (int c=0; c<char_blocksize/sizeof(complex<int16>); c++) {
	cout<<((complex<int16>*)&inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader])[c]<<" ";
      }
      cout<<endl;
  }      
}
