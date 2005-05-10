//#  WH_RSP.cc: Store delay-control synchronized RSP data in several 
//#             (NOWH_correlator) StationData dataholders
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
#include <WH_RSP.h>
#include <DH_RSP.h>
#include <DH_RSPSync.h>
#include <DH_StationData.h>

using namespace LOFAR;

ProfilingState WH_RSP::theirCatchingUpState;
ProfilingState WH_RSP::theirWaitingState;

WH_RSP::WH_RSP(const string& name, 
               const KeyValueMap kvm) 
  : WorkHolder (2,
		kvm.getInt("NoWH_Correlator", 7), 
		name, 
		"WH_RSP"),
    itsKVM (kvm),
    itsNextStamp(-1,0),
    itsReadNext(true)
{
  char str[32];
  
  // get parameters
  itsNCorrOutputs  = kvm.getInt("NoWH_Correlator", 7);  
  itsNRSPOutputs   = kvm.getInt("NoWH_RSP", 2) - 1;  
  itsPolarisations = kvm.getInt("polarisations",2);           
  itsNbeamlets     = kvm.getInt("NoRSPBeamlets", 92) / itsNCorrOutputs; // number of EPA-packet beamlets per OutDataholder
  itsNpackets      = kvm.getInt("NoPacketsInFrame", 8);                 // number of EPA-packets in RSP-ethernetframe
  itsSzEPAheader   = kvm.getInt("SzEPAheader", 14);                     // headersize in bytes
  itsSzEPApacket   = (itsPolarisations * sizeof(complex<int16>) * kvm.getInt("NoRSPBeamlets", 92)) + itsSzEPAheader; // packetsize in bytes

  // create incoming RSP dataholder    
  getDataManager().addInDataHolder(0, new DH_RSP("DH_RSP_in", itsKVM)); // buffer of char

  // create incoming Sync-timestamp dataholder
  getDataManager().addInDataHolder(1, new DH_RSPSync("DH_RSP_delay"));
  
  // create outgoing RSP dataholders
  int bufsize =  itsPolarisations * itsNbeamlets * itsNpackets;
  for (int i=0; i < itsNCorrOutputs; i++) {
    snprintf(str, 32, "DH_StationData_%d", i);
    getDataManager().addOutDataHolder(i, new DH_StationData(str, bufsize)); // buffer of complex<int16>   
  }

  // use cyclic buffer on RSP input
  //((DataManager)getDataManager()).setInBufferingProperties(0, false); 

  // do not use autotriggering on the RSP input
  // 'new read' trigger will be set manually
  getDataManager().setAutoTriggerIn(0, false); 

  // init profiling states
  theirCatchingUpState.init ("WH_RSP catching up", "yellow");
  theirWaitingState.init ("WH_RSP waitinge", "orange");
}

WH_RSP::~WH_RSP() {
}

WorkHolder* WH_RSP::construct(const string& name,
                              const KeyValueMap kvm) 
{
  return new WH_RSP(name, kvm);
}

WH_RSP* WH_RSP::make(const string& name)
{
  return new WH_RSP(name, itsKVM);
}

void WH_RSP::process() 
{ 
  DH_RSP* inDHp;
  DH_StationData* outDHp;
  SyncStamp thisStamp;
  bool newStamp = false;
  int char_blocksize; 
  int complex_int16_blocksize;
  int amountToCopy; 

  if (itsReadNext) {
    // read next packet from input
    getDataManager().getInHolder(0);
    getDataManager().readyWithInHolder(0);
  } 
  
  if (itsNextStamp.getSeqId() == -1) {
    // force to read the first delay_controlled timestamp
    getDataManager().getInHolder(1);
    getDataManager().readyWithInHolder(1);
  }
  // get delay_controlled timestamp
  DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
  itsNextStamp = dhp->getSyncStamp(); 
  // we need to increment the stamp because it is written only once per second or so
  dhp->incrementStamp(itsNpackets);
      
  while (!newStamp) {

    // get incoming RSP data
    inDHp = (DH_RSP*)getDataManager().getInHolder(0);
    thisStamp.setStamp(inDHp->getSeqID(), inDHp->getBlockID());

    // Use next line to bypass synchronization 
    //itsNextStamp = thisStamp;

    if (thisStamp < itsNextStamp) {
      // catching up: (current timestamp < delay_control timestamp)
      
      // set profiling state
      theirCatchingUpState.enter();
      theirWaitingState.leave();
      
      LOG_TRACE_COND_STR("Packet too old; skip");
      
      // stay in while loop and trigger a new read
      // so this packet will be skipped
      getDataManager().readyWithInHolder(0);
      newStamp = false;
    }
    else if (thisStamp > itsNextStamp) {
      // wait: (current timestamp > delay_control timestamp)
      
      // set profiling state
      theirCatchingUpState.leave();
      theirWaitingState.enter();
      
      // step out of the while loop and do not trigger a new read
      // so this packet will be read again next loop
      newStamp = true; 
      itsReadNext = false;       
    }
    else {
      // synchronized: current timestamp == delay_control timestamp
      
      // reset profiling states
      theirCatchingUpState.leave();
      theirWaitingState.leave();
      
      // copy stationID, blockID and valid/invalid flag of first EPA-packet in OutDataholder
      outDHp = (DH_StationData*)getDataManager().getOutHolder(0);
      outDHp->setStationID( inDHp->getStationID() );
      outDHp->setBlockID( inDHp->getBlockID() );
      outDHp->setFlag( inDHp->getFlag() );

       // valid data received?
      bool valid = !inDHp->getFlag();

      if (valid) { 
         // valid beamlets
        char_blocksize = itsNbeamlets * itsPolarisations * sizeof(complex<int16>); 
        complex_int16_blocksize = itsNbeamlets * itsPolarisations;
        amountToCopy = itsPolarisations * itsNbeamlets * sizeof(complex<int16>);
        for (int i=0;i<itsNpackets;i++) {
          for (int j=0;j<itsNCorrOutputs;j++) {
            outDHp = (DH_StationData*)getDataManager().getOutHolder(j);
            memcpy( &outDHp->getBuffer()[i * complex_int16_blocksize], 
	            &inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader + (j * char_blocksize)],
                    amountToCopy);
          }
        }
      }
      else { 
        // invalid beamlets
        amountToCopy = itsPolarisations * itsNbeamlets * itsNpackets * sizeof(complex<int16>);
        for (int i=0;i<itsNCorrOutputs;i++) {
         outDHp = (DH_StationData*)getDataManager().getOutHolder(i);
         memset (outDHp->getBuffer(), 0, amountToCopy);   
        }
      }
      
      // step out of the while loop and trigger a new read
      newStamp = true;
      itsReadNext = true;
    }
  } // while (!newStamp)
}


void WH_RSP::dump() {
  cout<<"DUMP OF WH_RSP: "<<getName()<<endl;
  DH_RSP* inDHp;
  DH_StationData* outDHp;
  inDHp = (DH_RSP*)getDataManager().getInHolder(0);
      
  // determine blocksizes of char-based InHolder and complex<int16-based> OutHolder
  int char_blocksize   = itsNbeamlets * itsPolarisations * sizeof(complex<int16>); 
  int complex_int16_blocksize = itsNbeamlets * itsPolarisations;
      
  for (int i=0;i<itsNpackets;i++) {
    for (int j=0;j<itsNCorrOutputs;j++) {
      outDHp = (DH_StationData*)getDataManager().getOutHolder(j);
      cout<<"packet: "<<i<<"  output: "<<j<<"   ";
      for (int c=0; c<char_blocksize/sizeof(complex<int16>); c++) {
	cout<<((complex<int16>*)&inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader + (j * char_blocksize)])[c]<<" ";
      }
      cout<<endl;
    }
  }      
} 
  
