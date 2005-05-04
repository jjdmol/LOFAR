//#  WH_RSP.cc: Store RSP beamlets, blockID, stationID and isValid flag in 
//#             in several (NOWH_correlator) StationData dataholders
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
#include <DH_StationData.h>

using namespace LOFAR;

ProfilingState WH_RSP::theirInvalidDataState;
ProfilingState WH_RSP::theirTransposeState;

WH_RSP::WH_RSP(const string& name, 
               const KeyValueMap kvm) 
  : WorkHolder (1,
		kvm.getInt("NoWH_Correlator", 7), 
		name, 
		"WH_RSP"),
    itsKVM (kvm)
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

  // create incoming dataholder   
  getDataManager().addInDataHolder(0, new DH_RSP("DH_RSP_in", itsKVM)); // buffer of char
  //((DataManager)getDataManager()).setOutBufferingProperties(i, false);
  
  // create outgoing dataholders
  int bufsize =  itsPolarisations * itsNbeamlets * itsNpackets;
  for (int i=0; i < itsNCorrOutputs; i++) {
    snprintf(str, 32, "DH_out_%d", i);
    getDataManager().addOutDataHolder(i, new DH_StationData(str, bufsize)); // buffer of complex<int16>
    
  }

  theirInvalidDataState.init ("WH_RSP invalid data", "yellow");
  theirTransposeState.init ("WH_RSP transpose", "orange");
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
  int char_blocksize; 
  int complex_int16_blocksize;
  int amountToCopy; 
      
  // get incoming dataholder
  inDHp = (DH_RSP*)getDataManager().getInHolder(0);

  // is data valid?
  bool invalid = inDHp->getFlag();

  // copy stationID, blockID and valid/invalid flag of first EPA-packet in OutDataholder
  outDHp = (DH_StationData*)getDataManager().getOutHolder(0);
  outDHp->setStationID( inDHp->getStationID() );
  outDHp->setBlockID( inDHp->getBlockID() );
  outDHp->setFlag( invalid );
      
  if ( invalid ) { // invalid beamlets
    theirTransposeState.leave();
    theirInvalidDataState.enter();
    amountToCopy = itsPolarisations * itsNbeamlets * itsNpackets * sizeof(complex<int16>);
    for (int i=0;i<itsNCorrOutputs;i++) {
      outDHp = (DH_StationData*)getDataManager().getOutHolder(i);
      memset (outDHp->getBuffer(), 0, amountToCopy);   
    }
  }
  else { // valid beamlets
    theirTransposeState.enter();
    theirInvalidDataState.leave();
    char_blocksize = itsNbeamlets * itsPolarisations * sizeof(complex<int16>); 
    complex_int16_blocksize = itsNbeamlets * itsPolarisations;
    amountToCopy = itsPolarisations * itsNbeamlets * sizeof(complex<int16>);
    for (int i=0;i<itsNpackets;i++) {
      for (int j=0;j<itsNCorrOutputs;j++) {
        outDHp = (DH_StationData*)getDataManager().getOutHolder(j);
        memcpy( &outDHp->getBuffer()[i * complex_int16_blocksize], 
	        &inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader + (j * char_blocksize)],
                amountToCopy);
#define DUMP_NOT_DEFINED
#ifdef DUMP
        cout<<"packet: "<<i<<"  output: "<<j<<"   ";
        for (int c=0; c<char_blocksize/sizeof(complex<int16>); c++) {
          cout<<((complex<int16>*)&inDHp->getBuffer()[(i * itsSzEPApacket)+ itsSzEPAheader + (j * char_blocksize)])[c]<<" ";
        }
        cout<<endl;
#endif
      }
    }
  }
  theirTransposeState.leave();
  theirInvalidDataState.leave();
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
  
