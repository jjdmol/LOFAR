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
//  Revision 1.3  2002/05/08 08:20:04  schaaf
//  Modified includes for new build env
//
//  Revision 1.2  2002/03/08 11:38:42  wierenga
//  Upgraded from firewalls.h use to Debug.h use. This version was used for performance tests.
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////


#include "P2Perf/DH_IntArray.h"
#include <Common/Debug.h>


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
