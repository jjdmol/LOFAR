//  DH_Correlations.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//
//////////////////////////////////////////////////////////////////////


#include "Pipeline/DH_Correlations.h"

DH_Correlations::DH_Correlations (const string& name,
				  short stations,
				  short channels,
				  short polarisations)
  : DataHolder (name, "DH_Correlations"),
    itsStations(stations),
    itsChannels(channels),
    itsPols(polarisations)
{
  DbgAssertStr(itsStations>0,"itsStations < 0");
  itsBaselines = stations*(stations+1) / 2;
  TRACER4("End of C'tor");
}

void DH_Correlations::preprocess(){
  // Create the DataPacket AND its buffer in contiguous memory
  
  // Determine the number of bytes needed for DataPacket and buffer.
  // the size is that of the DataPacket object, plus the size of the Buffer
  unsigned int size = sizeof(DataPacket) + ((itsBaselines*itsChannels*itsPols) * sizeof(DH_Correlations::DataType));
  cdebug(3) << "DataPacket size = " << size << endl;
  // allocate the memmory
  void* ptr = allocate(size+4); // extra 4 bytes to avoid problems with word allignment
  
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  
  // Initialize base class.
  setDataPacket (itsDataPacket, size);
  TRACER2("Created Correlations cube: " << itsDataPacket << "   size="  << size);
}

DH_Correlations::~DH_Correlations()
{
}


void DH_Correlations::reset() {
  TRACER4("Entering Reset");
  for (int stationA=0; stationA < itsStations; stationA++) {
    for (int stationB=0; stationB <= stationA; stationB++) {
      *getBuffer(stationA,stationB) = DataType(0); 
    }
  }
  TRACER4("Reset completed");
}
