//#  WH_RSP.cc: Analyse RSP ethernet frames and store datablocks, blockID, 
//#             stationID and isValid flag in DH_StationData
//*
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
#include <WH_RSP.h>
#include <DH_RSP.h>
#include <DH_RSPSync.h>
#include <DH_StationData.h>

using namespace LOFAR;

WH_RSP::WH_RSP(const string& name, 
               const KeyValueMap kvm,
	       const bool isSyncMaster) 
  : WorkHolder ((isSyncMaster ? 1 : 2), // if we are a syncSlave we have 2 inputs
		                        // else we have NoWH_RSP - 1 extra outputs:
		kvm.getInt("NoWH_Correlator", 7) + (isSyncMaster?kvm.getInt("NoWH_RSP", 2) - 1 : 0), 
		name, 
		"WH_RSP"),
    itsKVM (kvm),
    itsIsSyncMaster(isSyncMaster),
    itsNextStamp(0),
    itsDelay(0)
{
  char str[32];
  
  // get parameters
  itsNCorrOutputs  = kvm.getInt("NoWH_Correlator", 7);  
  itsNRSPOutputs   = kvm.getInt("NoWH_RSP", 2) - 1;  
  itsPolarisations = kvm.getInt("polarisations",2);           
  itsNbeamlets     = kvm.getInt("NoRSPbeamlets", 92) / itsNCorrOutputs; // number of EPA-packet beamlets per OutDataholder
  itsNpackets      = kvm.getInt("NoPacketsInFrame", 8);             // number of EPA-packets in RSP-ethernetframe
  itsSzEPAheader   = kvm.getInt("SzEPAheader", 14);                 // headersize in bytes
  itsSzEPApacket   = (8 * itsNbeamlets) + itsSzEPAheader;           // packetsize in bytes

  // create buffer for incoming dataholders 
  // implement a cyclic buffer later !!!
  getDataManager().addInDataHolder(0, new DH_RSP("DH_in", kvm.getInt("SzDH_RSP",6000))); // buffer of char
  
  // create outgoing dataholders
  int bufsize =  itsPolarisations * itsNbeamlets * itsNpackets;
  for (int i=0; i < itsNCorrOutputs; i++) {
    sprintf(str, "DH_out_%d", i);
    getDataManager().addOutDataHolder(i, new DH_StationData(str, bufsize)); // buffer of complex<uint16>
  }

  if (itsIsSyncMaster) {
    // if we are the sync master we need extra outputs (NoWH_RSP -1)
    for (int i = itsNCorrOutputs; i < itsNCorrOutputs + itsNRSPOutputs; i++) {
      sprintf(str, "DH_Sync_out_%d", i);
      getDataManager().addOutDataHolder(i, new DH_RSPSync(str));
    }
  } else {
    // if we are a sync slave we need 1 extra input
    getDataManager().addInDataHolder(1, new DH_RSPSync("DH_Sync_in"));
  }
  // We need to be able to read more than one packet at the time in case we are lagging
  // and we want to skip one or more packets.
  getDataManager().setAutoTriggerIn(0, false);
}

WH_RSP::~WH_RSP() {
}

WorkHolder* WH_RSP::construct(const string& name,
                              const KeyValueMap kvm,
			      const bool isSyncMaster) 
{
  return new WH_RSP(name, kvm, isSyncMaster);
}

WH_RSP* WH_RSP::make(const string& name)
{
  return new WH_RSP(name, itsKVM, itsIsSyncMaster);
}

void WH_RSP::process() 
{ 
  if (!itsIsSyncMaster) {
    // we are a slave so read the syncstamp
    DH_RSPSync* dhp = (DH_RSPSync*)getDataManager().getInHolder(1);
    itsNextStamp = dhp->getSyncStamp(); 
    itsNextStamp += itsDelay; 
    // we need to increment the stamp because it is written only once per second or so
    dhp->incrementStamp(itsNpackets);
  } else {
    // we are a master, so increase the nextValue and send it to the slaves
    if (itsNextStamp.getSeqId() == 0) {
      //this is the first loop
      // so take get current time stamp and wait 1 sec to 
      DH_RSP* inDHp = (DH_RSP*)getDataManager().getInHolder(0);
      itsNextStamp.setStamp(inDHp->getSeqID(), 0);
      itsNextStamp += itsNpackets * 10000; // at 1 Gb/s I think this is .5 sec
    } else {
      itsNextStamp += itsNpackets;
    }
    for (int i = itsNCorrOutputs; i < itsNCorrOutputs + itsNRSPOutputs; i++) {
      ((DH_RSPSync*)getDataManager().getOutHolder(i))->setSyncStamp(itsNextStamp);
    }
  }    

  DH_RSP* inDHp;
  DH_StationData* outDHp;
  bool inSync = false;
  SyncStamp* thisStamp;
  
  while (!inSync) {
    inDHp = (DH_RSP*)getDataManager().getInHolder(0);
    thisStamp->setStamp(inDHp->getSeqID(), inDHp->getBlockID());
    
    if (*thisStamp < itsNextStamp) {

      // this packets time stamp is too old, it should have been sent already
      // so do nothing and read again
      LOG_TRACE_COND_STR("Package too old; skip");
      getDataManager().readyWithInHolder(0);  

    } else if (itsNextStamp + (itsNpackets - 1) < *thisStamp) {

      LOG_TRACE_COND_STR("Package has been missed, insert dummy package");
      // we missed some packets, 
      // send a dummy and do NOT read again 
      // so this packet will be read again in the next process step
      
      // todo: insert the correct number of packets in one go

      // determine blocksizes of char-based InHolder and complex<uint16-based> OutHolder
      int char_blocksize   = itsNbeamlets * itsPolarisations * sizeof(complex<uint16>); 
      int complex_uint16_blocksize = itsNbeamlets * itsPolarisations;
      
      //todo: create an appropriate dummy packet only once and re-use.

      // copy stationID and blockID of first EPA-packet in OutDataholder
      outDHp = (DH_StationData*)getDataManager().getOutHolder(0);
      outDHp->setStationID( 0 );
      outDHp->setBlockID( 0 );
      outDHp->setFlag( 1 );
      
      // copy the beamlets from all EPA-packets in OutDataholder
      for (int i=0;i<itsNpackets;i++) {
	for (int j=0;j<itsNCorrOutputs;j++) {
	  outDHp = (DH_StationData*)getDataManager().getOutHolder(j);
          memset( &outDHp->getBuffer()[i * complex_uint16_blocksize], 
		  0,
		  char_blocksize );
	}
      }      
      // step out of the while loop and do not call readyWithInHolder
      inSync = true;
      
    } else { /* (*thisStamp == itsNextStamp) { */

      LOG_TRACE_COND_STR("Package has correct timestamp");
      // this is the right packet, so send it
      inSync = true;
      
      // determine blocksizes of char-based InHolder and complex<uint16-based> OutHolder
      int char_blocksize   = itsNbeamlets * itsPolarisations * sizeof(complex<uint16>); 
      int complex_uint16_blocksize = itsNbeamlets * itsPolarisations;
      
      // copy stationID and blockID of first EPA-packet in OutDataholder
      outDHp = (DH_StationData*)getDataManager().getOutHolder(0);
      outDHp->setStationID( inDHp->getStationID() );
      outDHp->setBlockID( inDHp->getBlockID() );
      outDHp->setFlag( 0 );
      
      // copy the beamlets from all EPA-packets in OutDataholder
      for (int i=0;i<itsNpackets;i++) {
	for (int j=0;j<itsNCorrOutputs;j++) {
          outDHp = (DH_StationData*)getDataManager().getOutHolder(j);
          memcpy( &outDHp->getBuffer()[i * complex_uint16_blocksize], 
		  &inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader + (j * char_blocksize)],
		  char_blocksize );
	}
      }      
      
      // Tell dataManager we are ready with InHolder, this will trigger a new read.
      // (this InHolder is not auto triggered)
      getDataManager().readyWithInHolder(0);        
    } 
  }
}
