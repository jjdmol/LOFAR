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


#include <DH_Beamlet.h>
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
  itsBuffer             (0),
  itsStationID          (StationID),
  itsFrequencyOffset    (FreqOff),
  itsChannelWidth       (channelWidth),
  itsElapsedTime        (ElapsedTime),
  itsNumberOfChannels   (nchan)
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nchan ;// * sizeof(BufferType);
}

  DH_Beamlet::DH_Beamlet (const string& name, 
			  const int nchan)
: DataHolder            (name, "DH_Beamlet"),
  itsBuffer             (0)
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nchan ;
}

DH_Beamlet::DH_Beamlet(const DH_Beamlet& that)
  : DataHolder           (that)
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = that.getNumberOfChannels() ;
}

DH_Beamlet::~DH_Beamlet()
{
  //  delete [] (char*)(itsDataPacket);
}

DataHolder* DH_Beamlet::clone() const
{
  return new DH_Beamlet(*this);
}

void DH_Beamlet::preprocess()
{
  addField("Buffer", BlobField<BufferType>(1,itsNumberOfChannels));

  createDataBlock();
  for (unsigned int i=0; i<itsBufSize; i++){ 
    itsBuffer[i] = 0;
  }
}

void DH_Beamlet::postprocess()
{
  itsBuffer            = 0;
}

void DH_Beamlet::fillDataPointers() {
  itsBuffer           = getData<BufferType> ("Buffer");
}

}
