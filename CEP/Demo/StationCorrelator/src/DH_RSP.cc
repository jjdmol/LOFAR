//  DH_RSP.cc:
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


#include <DH_RSP.h>

namespace LOFAR
{

DH_RSP::DH_RSP (const string& name, 
                const unsigned int bufsize)
: DataHolder (name, "DH_RSP"),
  itsBuffer  (0),
  itsBufSize (bufsize)
{}

DH_RSP::DH_RSP(const DH_RSP& that)
: DataHolder (that),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_RSP::~DH_RSP()
{}

DataHolder* DH_RSP::clone() const
{
  return new DH_RSP(*this);
}

void DH_RSP::preprocess()
{
  initDataFields();
  
  // Add the fields to the data definition.
  addField ("Buffer", BlobField<BufferType>(1,itsBufSize));

  createDataBlock();
  itsBuffer = getData<BufferType> ("Buffer");
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_RSP::setBufferSize (unsigned int bufsize)
{
  itsBufSize = bufsize;
  preprocess();
}

void DH_RSP::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

void DH_RSP::postprocess()
{
  itsBuffer = 0;
}

//DH_RSP::BufferType& DH_RSP::getBufferElement(unsigned int element)
//{
// ASSERT(element < itsBufSize); 
//return itsBuffer[element];
//}

} // end namespace
