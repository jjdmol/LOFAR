//  DH_IntArray.cc:
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


#include "DH_IntArray.h"
#include "firewalls.h"


DH_IntArray::DH_IntArray (const string& name, unsigned int nbuffer)
: DataHolder (name, "DH_IntArray")
{
  // Determine the number of bytes needed for DataPacket and buffer.
  unsigned int size = sizeof(DataPacket) + (nbuffer * sizeof(BufferType));
  char* ptr = new char[size];

  cout << "size = " << size << endl;
  cout << "sizeof(BufferType) = " << sizeof(BufferType) << endl;

  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();

  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  for (unsigned int i=0; i<nbuffer; i++) {
    itsBuffer[i] = 0;
  }
  // Initialize base class.
  setDataPacket (itsDataPacket, size);
}

DH_IntArray::~DH_IntArray()
{
  delete [] (char*)(itsDataPacket);
}
