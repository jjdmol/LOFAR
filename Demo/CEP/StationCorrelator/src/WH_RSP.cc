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
#include <DH_StationData.h>

using namespace LOFAR;


WH_RSP::WH_RSP(const string& name, 
               const KeyValueMap kvm) 
  : WorkHolder (1, kvm.getInt("NoWH_Correlator", 7), name, "WH_RSP"),
    itsKVM (kvm)
{
  char str[15];
  
  // get parameters
  itsNoutputs      = kvm.getInt("NoWH_Correlator", 7);  
  itsPolarisations = kvm.getInt("polarisations",2);           
  itsNbeamlets     = kvm.getInt("NoRSPbeamlets", 92) / itsNoutputs; // number of EPA-packet beamlets per OutDataholder
  itsNpackets      = kvm.getInt("NoPacketsInFrame", 8);             // number of EPA-packets in RSP-ethernetframe
  itsSzEPAheader   = kvm.getInt("SzEPAheader", 14);                 // headersize in bytes
  itsSzEPApacket   = (8 * itsNbeamlets) + itsSzEPAheader;           // packetsize in bytes

  // create buffer for incoming dataholders 
  // implement a cyclic buffer later !!!
  getDataManager().addInDataHolder(0, new DH_RSP("DH_in", kvm.getInt("SzDH_RSP",6000))); // buffer of char
  
  // create outgoing dataholders
  int bufsize =  itsPolarisations * itsNbeamlets * itsNpackets;
  for (int i=0; i < itsNoutputs; i++) {
    sprintf(str, "DH_out_%d", i);
    getDataManager().addOutDataHolder(i, new DH_StationData(str, bufsize)); // buffer of complex<uint16>
  }
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
  // get the RSP data frame from the Input dataholder
  const char* rspdata = static_cast<char*>(((DH_RSP*)getDataManager().getInHolder(0))->getBuffer());

  // determine blocksizes of char-based InHolder and complex<uint16-based> OutHolder
  int charblocksize   = itsNbeamlets * itsPolarisations * sizeof(complex<uint16>); 
  int uint16blocksize = itsNbeamlets * itsPolarisations;

  // copy stationID and blockID of first EPA-packet in OutDataholder
  ((DH_StationData*)getDataManager().getOutHolder(0))->setStationID( ((int*)&rspdata[2])[0] );
  ((DH_StationData*)getDataManager().getOutHolder(0))->setBlockID( ((int*)&rspdata[10])[0] );
  ((DH_StationData*)getDataManager().getOutHolder(0))->setFlag( 0 );

  // copy the beamlets from all EPA-packets in OutDataholder
  // implement code for if frames are missing later !!!
  for (int i=0;i<itsNpackets;i++) {
     for (int j=0;j<itsNoutputs;j++) {
       memcpy( &((DH_StationData*)getDataManager().getOutHolder(j))->getBuffer()[i * uint16blocksize], 
               &rspdata[(i * itsSzEPApacket)+ itsSzEPAheader + (j * charblocksize)], 
               charblocksize );
     }
  }
  
}
