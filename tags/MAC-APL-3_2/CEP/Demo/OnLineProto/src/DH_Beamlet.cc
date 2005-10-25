//  DH_Beamlet.cc:
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


#include "DH_Beamlet.h"
#include <Common/KeyValueMap.h>

namespace LOFAR
{

  DH_Beamlet::DH_Beamlet (const string& name, 
			  const int StationID, 
			  float FreqOff,
			  float channelWidth,
			  const float ElapsedTime, 
			  const int nchan)
: DataHolder            (name, "DH_Beamlet"),
  itsBufferptr          (0)
{
  itsStationID        = StationID;
  itsFrequencyOffset  = FreqOff;
  itsChannelWidth     = channelWidth;
  itsElapsedTime      = ElapsedTime;
  itsNumberOfChannels = nchan;
}

  DH_Beamlet::DH_Beamlet (const string& name, 
			  const int nchan)
: DataHolder            (name, "DH_Beamlet"),
  itsBufferptr          (0)
{
  itsStationID        = -1;
  itsFrequencyOffset  = -1;
  itsChannelWidth     = -1;
  itsElapsedTime      = -1;
  itsNumberOfChannels = nchan;
}

DH_Beamlet::DH_Beamlet(const DH_Beamlet& that)
  : DataHolder     (that),
    itsBufferptr   (0)
{
  // Shouldn't we test if the blob is already initialised??

   itsStationID        = that.itsStationID;
   itsFrequencyOffset  = that.itsFrequencyOffset;
   itsChannelWidth     = that.itsChannelWidth;
   itsElapsedTime      = that.itsElapsedTime;
   itsNumberOfChannels = that.itsNumberOfChannels;
}

DH_Beamlet::~DH_Beamlet()
{
}

DataHolder* DH_Beamlet::clone() const
{
  return new DH_Beamlet(*this);
}

void DH_Beamlet::preprocess()
{
  // Add the fields to the data definition.
  addField("StationID",BlobField<int>(1,1));
  addField("FreqOffset",BlobField<float>(1,1));
  addField("ElapsedTime",BlobField<float>(1,1));
  addField("NrOfChannels",BlobField<int>(1,1));
  addField("ChannelWidth",BlobField<float>(1,1));
  addField("Buffer", 
	   BlobField< complex<float> >(1, //version 
			      itsNumberOfChannels)); //no_elements
  // Create the data blob (which calls fillPointers).
  createDataBlock();

  // Initialize the blob fields.
  *itsStationIDptr        = itsStationID;
  *itsFrequencyOffsetptr  = itsFrequencyOffset;
  *itsChannelWidthptr     = itsChannelWidth;
  *itsElapsedTimeptr      = itsElapsedTime;
  *itsNumberOfChannelsptr = itsNumberOfChannels;
}

void DH_Beamlet::postprocess()
{
  itsBufferptr        = 0;
  itsStationID        = 0;
  itsFrequencyOffset  = 0;
  itsChannelWidth     = 0;
  itsElapsedTime      = 0;
  itsNumberOfChannels = 0;
}

void DH_Beamlet::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBufferptr           = getData<complex<float> > ("Buffer");
  itsStationIDptr        = getData<int>   ("StationID");
  itsFrequencyOffsetptr  = getData<float> ("FreqOffset");
  itsChannelWidthptr     = getData<float> ("ChannelWidth");
  itsElapsedTimeptr      = getData<float> ("ElapsedTime");
  itsNumberOfChannelsptr = getData<int>   ("NrOfChannels");

}

}//namespace
