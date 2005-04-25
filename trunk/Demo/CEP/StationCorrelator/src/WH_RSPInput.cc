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

using namespace LOFAR;

ProfilingState WH_RSPInput::theirCatchingUpState;
ProfilingState WH_RSPInput::theirWaitingState;

WH_RSPInput::WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
	                 const bool isSyncMaster) 
  : WorkHolder ((isSyncMaster ? 1 : 2), // if we are a syncSlave we have 2 inputs
		                        // else we have NoWH_RSP - 1 extra outputs:
		1 + (isSyncMaster?kvm.getInt("NoWH_RSP", 2) - 1 : 0), 
		name, 
		"WH_RSPInput"),
    itsKVM (kvm),
    itsIsSyncMaster(isSyncMaster),
    itsNextStamp(-1,0),
    itsDelay(0),
    itsReadNext(true)
{
  char str[32];
  
  // get parameters
  //itsNCorrOutputs  = kvm.getInt("NoWH_Correlator", 7);  
  itsNRSPOutputs   = kvm.getInt("NoWH_RSP", 2);  
  itsPolarisations = kvm.getInt("polarisations",2);           
  itsNbeamlets     = kvm.getInt("NoRSPBeamlets", 92) / itsNCorrOutputs; // number of EPA-packet beamlets per OutDataholder
  itsNpackets      = kvm.getInt("NoPacketsInFrame", 8);             // number of EPA-packets in RSP-ethernetframe
  itsSzEPAheader   = kvm.getInt("SzEPAheader", 14);                 // headersize in bytes
  itsSzEPApacket   = (itsPolarisations * sizeof(complex<int16>) * kvm.getInt("NoRSPBeamlets", 92)) + itsSzEPAheader; // packetsize in bytes

  // create incoming dataholder   
  getDataManager().addInDataHolder(0, new DH_RSP("DH_RSP_in", itsKVM));
  
  // create outgoing dataholder
  getDataManager().addOutDataHolder(0, new DH_RSP("DH_RSP_out", itsKVM));
  ((DataManager)getDataManager()).setOutBufferingProperties(0, false); // use CyclicBuffer on output


  if (itsIsSyncMaster) {
    // if we are the sync master we need extra outputs (NoWH_RSP -1)
    for (int i = 1; i < itsNRSPOutputs; i++) {
      snprintf(str, 32, "DH_Sync_out_%d", i);
      getDataManager().addOutDataHolder(i, new DH_RSPSync(str));
    }
  } else {
    // if we are a sync slave we need 1 extra input
    getDataManager().addInDataHolder(1, new DH_RSPSync("DH_Sync_in"));
  }
  // We need to be able to read more than one packet at the time in case we are lagging
  // and we want to skip one or more packets.
  getDataManager().setAutoTriggerIn(0, false); 

  theirCatchingUpState.init ("WH_RSPInput catching up", "yellow");
  theirWaitingState.init ("WH_RSPInput waiting", "orange");
}

WH_RSPInput::~WH_RSPInput() {
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


void WH_RSPInput::process() 
{ 
  if (itsReadNext) {
    getDataManager().getInHolder(0);
    getDataManager().readyWithInHolder(0);
  }   

  if (!itsIsSyncMaster) {
    // we are a slave so read the syncstamp
    if (itsNextStamp.getSeqId() == -1) {
      // this is the first time
      // we need to force a read, because this inHolder has a lower data rate
      // if the rate difference is 1000, the workholder will read for
      // the first time in the 1000th runstep.
      getDataManager().getInHolder(1);
      getDataManager().readyWithInHolder(1);
    }      
    
    DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
    itsNextStamp = dhp->getSyncStamp(); 
    itsNextStamp += itsDelay; 
    // we need to increment the stamp because it is written only once per second or so
    dhp->incrementStamp(itsNpackets);
  } 
  else { //(itsIsSyncMaster)
    // we are a master, so increase the nextValue and send it to the slaves
    if (itsNextStamp.getSeqId() == -1) {
      //this is the first loop
      // so take get current time stamp and determine the time stamp at which we all will start sending
      DH_RSP* inDHp = (DH_RSP*)getDataManager().getInHolder(0);
      itsNextStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());
      //      cout<<"first time in WH_RSP, timeStamp:"<<inDHp->getSeqID()<<endl;
      if (!itsKVM.getBool("useRealRSPBoard", false)) {
	// we are not using the real rspboards, so no delay
	;
      } else {
	// we are using the real rsp boards, put in a delay to
	// let the other WH_RSPs catch up
	itsNextStamp += itsNpackets * 1500; // right now 0.5 second
      }
      for (int i = 1; i < itsNRSPOutputs; i++) {
	((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsNextStamp);
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
  } //end (itsIsSyncMaster)    

  DH_RSP* inDHp;
  DH_RSP* outDHp;
  bool inSync = false;
  SyncStamp thisStamp;
  int amountToCopy =  itsNbeamlets * itsPolarisations * sizeof(complex<int16>);

  while (!inSync) {
    
    inDHp = (DH_RSP*)getDataManager().getInHolder(0);
    thisStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());
    //cout<<"expected: "<<itsNextStamp<<" received: "<<thisStamp<<endl;

    // Use the next line to bypass synchonization
#define NO_SYNC_NOT_DEFINED
#ifdef NO_SYNC
    thisStamp=itsNextStamp;
#endif
    
    if (!itsKVM.getBool("useRealRSPBoard", false)) {
      // we are not using the real rspboards, so no delay
      //  Do this only to bypass the syncronisation
      thisStamp=itsNextStamp;
    }

    if (thisStamp < itsNextStamp) { 
      theirCatchingUpState.leave();
      theirWaitingState.enter();
      //cout<<"packet with timestamp: "<<thisStamp<<" is too old! expected was: "<<itsNextStamp<<endl;

      // this packets time stamp is too old, it should have been sent already
      // so do nothing and read again
      LOG_TRACE_COND_STR("Package too old; skip");
      getDataManager().readyWithInHolder(0);
      inSync = false;
    } 
    else if (itsNextStamp + (itsNpackets - 1) < thisStamp) {
      theirWaitingState.leave();
      theirCatchingUpState.enter();
      //cout<<"packet with timestamp: "<<itsNextStamp<<" was missed! received: "<<thisStamp<<endl;

      LOG_TRACE_COND_STR("Package has been missed, insert dummy package");
      // we missed some packets, 
      // send a dummy and do NOT read again 
      // so this packet will be read again in the next process step
      
      // todo: insert the correct number of packets in one go

      //todo: create an appropriate dummy packet only once and re-use.

      // copy dummy data in OutDataholder 
      outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
      outDHp->setFlag( 1 ); // mark data as invalid
      memset( outDHp->getBuffer(), 0, amountToCopy);
      
      // step out of the while loop and do not call readyWithInHolder
      inSync = true;
      itsReadNext = false; // read this packet again next process      
    } 
    else { // (*thisStamp == itsNextStamp) 
      theirCatchingUpState.leave();
      theirWaitingState.leave();
      //cout<<"packet with timestamp: "<<thisStamp<<"was received correctly"<<endl;

      LOG_TRACE_COND_STR("Package has correct timestamp");
      // this is the right packet, so send it

      // copy contents of InDataHolder to OutDataholder
      outDHp = (DH_RSP*)getDataManager().getOutHolder(0);
      outDHp->setFlag( 0 ); // mark data as valid
      memcpy(outDHp->getBuffer(), inDHp->getBuffer(), amountToCopy);
      
#define DUMP_NOT_DEFINED 
#ifdef DUMP
     int char_blocksize = itsNbeamlets * itsPolarisations * sizeof(complex<int16>)
     for (int i=0;i<itsNpackets;i++) {
       cout<<"packet: "<<i<<"   ";
       for (int c=0; c<char_blocksize/sizeof(complex<int16>); c++) {
	 cout<<((complex<int16>*)&inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader])[c]<<" ";
       }
       cout<<endl;
     }
#endif
      
      // Tell dataManager we are ready with InHolder, this will trigger a new read.
      // (this InHolder is not auto triggered)
      itsReadNext = true;
      inSync = true;
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
