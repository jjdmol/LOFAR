//  DH_Vis.cc:
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


#include <DH_Vis.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_Vis::DH_Vis (const string& name)
: DataHolder    (name, "DH_Vis"),
  itsDataPacket (0),
  itsBuffer     (0)
{

}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder(that),
    itsDataPacket(0),
    itsBuffer(0)
{
}

DH_Vis::~DH_Vis()
{
  delete [] (char*)(itsDataPacket);
}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = NSTATIONS * NSTATIONS * sizeof(BufferType);
  unsigned int size = sizeof(DataPacket) + itsBufSize;
  char* ptr = new char[size];
  // Fill in the data packet pointer and initialize the memory.
  itsDataPacket = (DataPacket*)(ptr);
  *itsDataPacket = DataPacket();
  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));

  // Initialize base class.
  setDataPacket (itsDataPacket, size);
}

void DH_Vis::postprocess()
{
  delete [] (char*)(itsDataPacket);
  itsDataPacket = 0;
  itsBuffer     = 0;
  setDefaultDataPacket();
}

}
