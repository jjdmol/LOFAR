//  DH_Ethernet.cc:
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

#ifndef USE_NO_TH_ETHERNET

#include <lofar_config.h>
#include "DH_Ethernet.h"

namespace LOFAR
{

DH_Ethernet::DH_Ethernet (const string& name, 
                          unsigned int initialNelements,
			  bool useExtra)
: DataHolder (name, "DH_Ethernet", 1),
  itsBuffer  (0),
  itsBufSize (initialNelements)
{
  if (useExtra) {
  setExtraBlob ("Extra", 1);
  }
}

DH_Ethernet::DH_Ethernet(const DH_Ethernet& that)
: DataHolder (that),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_Ethernet::~DH_Ethernet()
{
  itsBuffer = 0;
}

DataHolder* DH_Ethernet::clone() const
{
  return new DH_Ethernet(*this);
}

void DH_Ethernet::init()
{
  initDataFields();
  
  // Add the fields to the data definition.
  addField ("Buffer", BlobField<BufferType>(1,itsBufSize));

  // Create the data blob (which calls fillPointers).
  createDataBlock();
  
  // Initialize the buffer.
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Ethernet::setBufferSize (unsigned int nelements)
{
  itsBufSize = nelements;
  init();
}

void DH_Ethernet::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

DH_Ethernet::BufferType& DH_Ethernet::getBufferElement(unsigned int element)
{
  ASSERT(element < itsBufSize); 
  return itsBuffer[element];
}

} // end namespace

#endif
