//#  WH_Transpose.cc: MPI_Alltoall transpose
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <WH_Transpose.h>
#include <DH_CorrCube.h>
#include <DH_StationData.h>

using namespace LOFAR;

WH_Transpose::WH_Transpose(const string& name, 
			   const KeyValueMap kvm) 
  : WorkHolder(kvm.getInt("NoWH_RSP",2), 1, name, "WH_Transpose"),
    itsKVM (kvm)
{
  char str[128];
  
  itsNstations         = itsKVM.getInt("stations", 2);
  itsNcorrelators      = itsKVM.getInt("NoWH_Correlator", 7);
  itsNsamples          = itsKVM.getInt("samples", 256000);
  itsNchannels         = itsKVM.getInt("NoRSPBeamlets", 92)/itsKVM.getInt("NoWH_Correlator", 92);
  itsNpolarisations    = itsKVM.getInt("polarisations", 2);
  itsNbeamletsinpacket = itsKVM.getInt("NoRSPbeamlets", 92);
  itsNpacketsinframe   = itsKVM.getInt("NoPacketsInFrame", 8);

  int bufsize = (itsNbeamletsinpacket / itsNcorrelators) * itsNpolarisations * itsNpacketsinframe;

  itsNinputs = itsKVM.getInt("noWH_RSP", 2);
  itsNoutputs = 1; // there is one connection to the corresponding WH_Correlator


  for (int i = 0; i < itsNinputs; i++) {
    snprintf(str, 128, "input_%d_of_%d", i, itsNinputs);
    getDataManager().addInDataHolder(i, new DH_StationData(str, bufsize));
  }
  for (int i = 0; i < itsNoutputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNoutputs);
    getDataManager().addOutDataHolder(i, new DH_CorrCube(str, 
							 itsNstations, 
							 itsNsamples, 
							 itsNchannels, 
							 itsNpolarisations));
  }
}

WH_Transpose::~WH_Transpose() {
}

WorkHolder* WH_Transpose::construct(const string& name, KeyValueMap kvm) {
  return new WH_Transpose(name, kvm);
}

WH_Transpose* WH_Transpose::make(const string& name) {
  return new WH_Transpose(name, itsKVM);
}

void WH_Transpose::process() {
  // note that this transpose only works for our initial demo of two RSP boards
  // it is not generic enough to handle other configurations

  DH_CorrCube* myDH  = static_cast<DH_CorrCube*> (getDataManager().getOutHolder(0));

  // first check if the blockID's of the Input dataHolders match (this should not fail)
  if ( static_cast<DH_StationData*>(getDataManager().getInHolder(0))->getBlockID() ==
       static_cast<DH_StationData*>(getDataManager().getInHolder(1))->getBlockID() ) {
    myDH->setBlockID(static_cast<DH_StationData*>(getDataManager().getInHolder(0))->getBlockID());
  } else {
    // something nasty happened
    
  }

  // set the flag of the outDH as the bitwise OR of the flags of the input DHs
  if ( (static_cast<DH_StationData*>(getDataManager().getInHolder(0))->getFlag() | static_cast<DH_StationData*>(getDataManager().getInHolder(1))->getFlag()) != 0 ) {
    myDH->setFlag( static_cast<DH_StationData*>(getDataManager().getInHolder(0))->getFlag() |
		   static_cast<DH_StationData*>(getDataManager().getInHolder(1))->getFlag() );
  }

  DH_StationData::BufferType* val_ptr_0 = static_cast<DH_StationData*>(getDataManager().getInHolder(0))->getBuffer();
  DH_StationData::BufferType* val_ptr_1 = static_cast<DH_StationData*>(getDataManager().getInHolder(1))->getBuffer();

  int offset = 0;

  for (int sample = 0; sample < itsNpacketsinframe; sample++) {
    //offset += itsNpolarisations + itsNbeamletsinpacket;
    for (int channel = 0; channel < itsNchannels; channel++) {
      //offset += itsNpolarisations;
      for (int polarisation = 0; polarisation < itsNpolarisations; polarisation++) {
	myDH->setBufferElement(channel, 
			       sample, 
			       0,
			       polarisation,
			       val_ptr_0+offset);
	myDH->setBufferElement(channel, 
			       sample, 
			       1,
			       polarisation,
			       val_ptr_1+offset);
	offset++;
      }
    }
  }
}
