//  DH_GrowSize.cc:
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
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//
//
//////////////////////////////////////////////////////////////////////


#include "DH_GrowSize.h"
#include "Debug.h"

DH_GrowSize::DH_GrowSize (const string& name, unsigned int nbuffer)
: DataHolder (name, "DH_GrowSize")
{
  // Determine the number of bytes needed for DataPacket and buffer.
  unsigned int size = sizeof(DataPacket) + (nbuffer * sizeof(BufferType));
  char* ptr = new char[size];

  cout << "nbuffer = " << nbuffer;
  cout << "DH_GrowSize buffersize = " << size << endl;
  //cout << "sizeof(BufferType) = " << sizeof(BufferType) << endl;
  //cout << "sizeof(DataPacket) = " << sizeof(DataPacket) << endl;

  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();

  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  for (unsigned int i=0; i<nbuffer; i++) {
    itsBuffer[i] = reportedDataPacketSize + sizeof(DataPacket);
  }
  // Initialize base class.
  setDataPacket (itsDataPacket, size);

  reportedDataPacketSize = 8; // initial size
  floatDataPacketSize = (int)reportedDataPacketSize;
}

DH_GrowSize::~DH_GrowSize()
{
  delete [] (char*)(itsDataPacket);
}
