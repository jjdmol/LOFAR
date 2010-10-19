//  DH_Phase.cc:
//
//  Copyright (C) 2004
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
//
//////////////////////////////////////////////////////////////////////


#include <DH_Phase.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

  DH_Phase::DH_Phase (const string& name, 
		      const int StationID)
: DataHolder            (name, "DH_Phase")
{
  itsStationID        = StationID;
  itsElapsedTime      = -1;
}

DH_Phase::DH_Phase(const DH_Phase& that)
  : DataHolder     (that)
{
  // todo: do we want to copy the blob avlues here or the temp. values from the DH?
  itsStationID        = that.itsStationID;
  itsElapsedTime      = that.itsElapsedTime;
}

DH_Phase::~DH_Phase()
{
}

DataHolder* DH_Phase::clone() const
{
  return new DH_Phase(*this);
}

void DH_Phase::preprocess()
{
  // Add the fields to the data definition.
  addField("ElapsedTime",BlobField< float >(1,1));
  addField("StationID",BlobField< int >(1,1));
  addField("Phase",BlobField< float >(1,1));
  addField("Buffer",BlobField< complex<float> >(1,1));
  

  // Create the data blob (which calls fillPointers).
  createDataBlock();

  // initialise the blob field with values from C'tor
  *itsStationIDptr        = itsStationID;
  *itsElapsedTimeptr      = itsElapsedTime;
  
}

void DH_Phase::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsPhaseptr          = getData<float> ("Phase");
  itsElapsedTimeptr    = getData<float> ("ElapsedTime");
  itsStationIDptr      = getData<int> ("StationID");
  itsBufferptr         = getData<complex<float> > ("Buffer");
}

void DH_Phase::postprocess()
{
}

}
