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
//  $Log$
//  Revision 1.7  2002/05/08 14:28:37  wierenga
//  DataHolder allocation moved from constructor to preprocess to be able to
//  use TransportHolder::allocate.
//  Bug fixes in P2Perf.cc for -mpi arguments.
//
//  Revision 1.6  2002/05/08 08:20:04  schaaf
//  Modified includes for new build env
//
//  Revision 1.5  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.4  2002/03/19 16:34:57  schaaf
//  reverted to version 1.2
//
//  Revision 1.3  2002/03/08 11:38:42  wierenga
//  Upgraded from firewalls.h use to Debug.h use. This version was used for performance tests.
//
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//
//
//////////////////////////////////////////////////////////////////////


#include "P2Perf/DH_GrowSize.h"
#include "Common/Debug.h"

/** 
    In the DH_Growsize constructor memory is allocated for the maximum 
    allowed buffer size. the CurDataPacketSize is set to a default
    determined by the nbuffer argument.
 */
DH_GrowSize::DH_GrowSize (const string& name, unsigned int nbuffer)
  : DataHolder (name, "DH_GrowSize"),
    itsBufSize(nbuffer),
    itsDataPacket(0)
{
}

DH_GrowSize::~DH_GrowSize()
{
}

void DH_GrowSize::preprocess()
{
  // Create the DataPacket AND its buffer in contiguous memory

  // Determine the number of bytes needed for DataPacket and buffer.
  // the size is that of the DataPacket object, plus the size of the Buffer
  unsigned int size = sizeof(DataPacket) + (itsBufSize * sizeof(BufferType));
  // allocate the memmory
  char* ptr = (char*)allocate(size);
  
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();

  // Fill in the buffer pointer and initialize the buffer.
  // the buffer starts after the DataPacket object
  itsDataPacket->itsBuffer = (BufferType*)(ptr + sizeof(DataPacket)); 
  // fill with zeroes
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsDataPacket->itsBuffer[i] = 0;
  }
  
  // Initialize base class.
  setDataPacket (itsDataPacket, size);

  // set initial reported datapacket size to zero buffer length
  reportedDataPacketSize = (float) sizeof(DataPacket); 
}

void DH_GrowSize::postprocess()
{
  // delete the allocated memmory for the DataPacket object, 
  // including the buffer
  deallocate((void*)itsDataPacket);
}
