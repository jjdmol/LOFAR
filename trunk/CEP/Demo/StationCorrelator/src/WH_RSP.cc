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
               const int elements) 
  : WorkHolder (1, 1, name, "WH_RSP"),
    itsInElements (elements)
{

  // Create cyclic buffer for incoming dataholders 
  getDataManager().addInDataHolder(0, new DH_RSP("in", elements));  // where to add bool synchronous = false???
  
  // Create outgoing dataholder
  getDataManager().addOutDataHolder(0, new DH_StationData("out"));
}

WH_RSP::~WH_RSP() {
}

WorkHolder* WH_RSP::construct(const string& name,
                              const int elements) 
{
  return new WH_RSP(name, elements);
}

WH_RSP* WH_RSP::make(const string& name)
{
  return new WH_RSP(name, itsInElements);
}

void WH_RSP::process() {

  // DH_RSP contains 8 EPA packets
  //   Format 1 EPA-packet:
  //   uint16 versionID | uint32 stationID | uint32 intervalID | uint32 blockID | EPA-data...
  //      EPA-data contains 92 beamlets
  //        Format 1 beamlet:
  //        uint16 Xr | uint16 Xi | uint16 Yr | uint16 Yi

  // Pseudocode to define what to do here:
 
  // if (!MissingFrames) {
  //   Get StationID
  //   Get BlockID of EPA-PACKET(0)
  //   Set InValid flag = FALSE 
  //   Copy EPA-data, StationID, BlockID and flag from DH_RSP to DH_StationData
  // }
  // else {
  //   do (for all missing frames) {
  //     Set InValid flag = TRUE
  //     Copy blank data in DH_StationData
  //   }
  // }

  //Example code:
  
  // itsStationID = ((DH_RSP*)getDataManager().getInHolder(0))->getBuffer()[1]
  // itsBlockID   = ((DH_RSP*)getDataManager().getInHolder(0))->getBuffer()[5]
}
