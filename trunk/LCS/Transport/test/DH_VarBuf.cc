//  DH_VarBuf.cc:
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
#include <Common/lofar_vector.h>
#include "DH_VarBuf.h"

namespace LOFAR
{

DH_VarBuf::DH_VarBuf (const string& name)
: DataHolder (name, "DH_VarBuf", 1),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (0)
{}

DH_VarBuf::DH_VarBuf(const DH_VarBuf& that)
: DataHolder (that),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_VarBuf::~DH_VarBuf()
{}

DataHolder* DH_VarBuf::clone() const
{
  return new DH_VarBuf(*this);
}

void DH_VarBuf::init()
{
  // Initialize the fieldset.
  initDataFields();
  // Add the fields to the data definition.
  addField ("Counter", BlobField<int>(1));
  addField ("Buffer", BlobField<BufferType>(1, //version 
					    itsBufSize)); //no_elements
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffer.
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = makefcomplex(0,0);
  }
}

void DH_VarBuf::setBufferSize (unsigned int nelements)
{
  itsBufSize = nelements;
  vector<uint> sizeVec(1);
  sizeVec[0] = itsBufSize;
  getDataField("Buffer").setShape(sizeVec);    // Adjust shape of datafield
  createDataBlock();
  for (unsigned int i=0; i<itsBufSize; i++)
  {
    itsBuffer[i] = makefcomplex(0,0);
  }
}

unsigned int DH_VarBuf::getBufferSize()
{ 
  vector<uint> sizeVec;
  sizeVec = getDataField("Buffer").getShape();
  itsBufSize = sizeVec[0];
  return itsBufSize;
}

void DH_VarBuf::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

DH_VarBuf::BufferType& DH_VarBuf::getBufferElement(unsigned int element)
{
  getBufferSize();
  ASSERT(element < itsBufSize); 
  return itsBuffer[element];
}

} // end namespace
