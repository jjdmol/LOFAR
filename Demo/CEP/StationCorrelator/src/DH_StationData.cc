//#  DH_StationData.cc
//#
//#  Copyright (C) 2002-2004
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

#include <DH_StationData.h>

using namespace LOFAR;


DH_StationData::DH_StationData (const string& name,
                                const unsigned int bufsize)
  : DataHolder     (name, "DH_StationData"),
    itsBufSize     (bufsize),
    itsBuffer      (0)
{ }

DH_StationData::~DH_StationData() 
{
  itsBuffer = 0;
}

DH_StationData::DH_StationData(const DH_StationData& that)
  : DataHolder(that),
    itsBuffer(0),
    itsBufsize(that.itsBufsize)
{ }

DataHolder* DH_StationData:: clone() const {
  return new DH_StationData(*this);
}

void DH_StationData::preprocess() {
  
  // first delete possible preexisting buffers
  postprocess();
                        
  // Add the fields to the data definition
  addField("StationID", BlobField<int>(1, 1));
  addField("BlockID", BlobField<int>(1, 1));
  addField("Flag", BlobField<int>(1, 1));
  addField("Buffer", BlobField<BufferType>(1, itsBufSize));

  // Create the data blob
  createDataBlock();

  // use memset to null the buffer instead of a for loop
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
}

void DH_StationData::postprocess() 
{
  itsBuffer = 0;
}

void DH_StationData::fillDataPointers() 
{  
  itsStationIDptr = getData<int> ("StationID");
  itsBlockIDptr   = getData<int> ("BlockID");
  itsFlagptr      = getData<int> ("Flag"); 
  itsBuffer       = getData<BufferType> ("Buffer");
}
