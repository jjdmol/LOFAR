//  DH_GrowSize.cc: DataHolder with one dimensional byte array that 
//                  can grow its size (e.g. for performance measurements)
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
//
//////////////////////////////////////////////////////////////////////


#include "AsyncTest/DH_GrowSize.h"
#include "Common/Debug.h"

/** 
    In the DH_Growsize constructor memory is allocated for the maximum 
    allowed buffer size. the CurDataPacketSize is set to a default
    determined by the nbuffer argument.
 */
DH_GrowSize::DH_GrowSize (const string& name, unsigned int nbuffer,
			  bool sizeFixed)
  : DataHolder (name, "DH_GrowSize"),
    itsBufSize(nbuffer),
    itsDataPacket(0),
    itsSizeFixed(sizeFixed)
{
}

DH_GrowSize::DH_GrowSize (const DH_GrowSize& that)
  : DataHolder(that),
    itsBufSize(that.itsBufSize),
    itsSizeFixed(that.itsSizeFixed),
    reportedDataPacketSize(that.reportedDataPacketSize)
{
}

DH_GrowSize::~DH_GrowSize()
{
}

DataHolder* DH_GrowSize::clone() const
{
  return new DH_GrowSize(*this);
}

void DH_GrowSize::preprocess()
{
  // Create the DataPacket AND its buffer in contiguous memory

  // Determine the number of bytes needed for DataPacket and buffer.
  // the size is that of the DataPacket object, plus the size of the Buffer
  unsigned int size = sizeof(DataPacket) + (itsBufSize * sizeof(BufferType));
  // allocate the memory
  char* ptr = (char*)allocate(size);
  
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();

  // Fill in the buffer pointer and initialize the buffer.
  // the buffer starts after the DataPacket object
  itsDataPacket->itsBuffer = (BufferType*)(ptr + sizeof(DataPacket)); 
  // fill with zeroes
  for (int i=0; i<itsBufSize; i++) {
    itsDataPacket->itsBuffer[i] = 0;
  }
  
  // Initialize base class.
  setDataPacket (itsDataPacket, size);

  if (!itsSizeFixed)
  {
    // set initial reported datapacket size to zero buffer length
    reportedDataPacketSize = (float) sizeof(DataPacket); 
  }
}

void DH_GrowSize::postprocess()
{
  // delete the allocated memory for the DataPacket object, 
  // including the buffer
  deallocate((void*)itsDataPacket);
}

void DH_GrowSize::dump()
{
}
