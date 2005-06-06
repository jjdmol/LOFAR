//  DH_Buffer.cc:
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

#include <lofar_config.h>
#include <AsyncTest/DH_Buffer.h>

namespace LOFAR
{

DH_Buffer::DH_Buffer (const string& name, unsigned int initialNelements)
: DataHolder (name, "DH_Buffer", 1),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (initialNelements)
{}

DH_Buffer::DH_Buffer(const DH_Buffer& that)
: DataHolder (that),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_Buffer::~DH_Buffer()
{}

DH_Buffer* DH_Buffer::clone() const
{
  return new DH_Buffer(*this);
}

void DH_Buffer::init()
{
  // Initialize the fieldset.
  initDataFields();
  // Add the fields to the data definition.
  addField("Counter", BlobField<int>(1));
  addField ("Buffer", BlobField<char>(1, //version 
				      itsBufSize)); //no_elements
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffer.
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Buffer::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer  = getData<char> ("Buffer");
}

char DH_Buffer::getBufferElement(unsigned int element)
{
  ASSERT(element < itsBufSize); 
  return itsBuffer[element];
}

} // end namespace
