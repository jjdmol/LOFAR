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
  itsDataPacket         (0),
  itsBuffer             (0)
{
  // ToDo: move allocation to preprocess

  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nchan * sizeof(BufferType);
  unsigned int size = sizeof(DataPacket) + itsBufSize;
  char* ptr = new char[size];
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  // Initialize base class.
  setDataPacket (itsDataPacket, size);

  itsDataPacket->itsStationID        = StationID;
  itsDataPacket->itsFrequencyOffset  = FreqOff;
  itsDataPacket->itsChannelWidth     = channelWidth;
  itsDataPacket->itsElapsedTime      = ElapsedTime;
  itsDataPacket->itsNumberOfChannels = nchan;
}

  DH_Beamlet::DH_Beamlet (const string& name, 
			  const int nchan)
: DataHolder            (name, "DH_Beamlet"),
  itsDataPacket         (0),
  itsBuffer             (0)
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nchan * sizeof(BufferType);
  unsigned int size = sizeof(DataPacket) + itsBufSize;
  char* ptr = new char[size];
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  // Initialize base class.
  setDataPacket (itsDataPacket, size);
  itsDataPacket->itsStationID        = -1;
  itsDataPacket->itsFrequencyOffset  = -1;
  itsDataPacket->itsChannelWidth     = -1;
  itsDataPacket->itsElapsedTime      = -1;
  itsDataPacket->itsNumberOfChannels = nchan;
}

DH_Beamlet::DH_Beamlet(const DH_Beamlet& that)
  : DataHolder     (that),
    itsDataPacket  (0),
    itsBuffer      (0)
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = that.getNumberOfChannels() * sizeof(BufferType);
  unsigned int size = sizeof(DataPacket) + itsBufSize;
  char* ptr = new char[size];
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  // Initialize base class.
  setDataPacket (itsDataPacket, size);
  itsDataPacket->itsStationID        = that.getStationID();
  itsDataPacket->itsFrequencyOffset  = that.getFrequencyOffset();
  itsDataPacket->itsChannelWidth     = that.getChannelWidth();
  itsDataPacket->itsElapsedTime      = that.getElapsedTime();
  itsDataPacket->itsNumberOfChannels = that.getNumberOfChannels();
}

DH_Beamlet::~DH_Beamlet()
{
  delete [] (char*)(itsDataPacket);
}

DataHolder* DH_Beamlet::clone() const
{
  return new DH_Beamlet(*this);
}

void DH_Beamlet::preprocess()
{
}

void DH_Beamlet::postprocess()
{
  delete [] (char*)(itsDataPacket);
  itsDataPacket = 0;
  itsBuffer     = 0;
  setDefaultDataPacket();
}

}
