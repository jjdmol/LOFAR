//#  WH_RSPBoard.cc: Analyse RSP ethernet frames and store datablocks, blockID, 
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
#include <WH_RSPBoard.h>
#include <DH_RSP.h>
#include <DH_RSPSync.h>
#include <DH_StationData.h>

using namespace LOFAR;

WH_RSPBoard::WH_RSPBoard(const string& name, 
			 const KeyValueMap kvm)
  : WorkHolder (0,
		kvm.getInt("NoWH_RSP", 2),
		name, 
		"WH_RSPBoard"),
    itsKVM (kvm),
    itsStamp(2, 0)
{
  char str[32];

  for (int i=0; i<itsNoutputs; i++) {
    sprintf(str, "DH_Boardout_%d2", i);
    getDataManager().addOutDataHolder(i, new DH_RSP(str, itsKVM)); // buffer of char
  }  
}

WH_RSPBoard::~WH_RSPBoard() {
}

WorkHolder* WH_RSPBoard::construct(const string& name,
                              const KeyValueMap kvm)
{
  return new WH_RSPBoard(name, kvm);
}

WH_RSPBoard* WH_RSPBoard::make(const string& name)
{
  return new WH_RSPBoard(name, itsKVM);
}

void WH_RSPBoard::process() 
{
//   cout<<"Stamp: "<<itsStamp<<endl;
  int noEP = itsKVM.getInt("NoPacketsInFrame", 8);
  int noBeamlet = itsKVM.getInt("NoRSPBeamlets", 92);
  for (int i=0; i<itsNoutputs; i++) {
    DH_RSP* outDH = (DH_RSP*) getDataManager().getOutHolder(i);
    outDH->resetBuffer();
    // These should be done per EPApacket not per frame
    outDH->setStationID(i);
    outDH->setSeqID(itsStamp.getSeqId());
    outDH->setBlockID(itsStamp.getBlockId());    
    for (int ep=0; ep<noEP; ep++) {
      for (int bl=0; bl<noBeamlet; bl++) {
	outDH->setBufferElement(ep, bl, 0, complex<uint16>(100*i+bl, 0));
      }
    }
  }

  itsStamp++;
}
