//  DH_Example.cc:
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


#include <DH_Example.h>

namespace LOFAR
{

DH_Example::DH_Example (const string& name, unsigned int nbuffer)
  : DataHolder (name, "DH_Example"),
    itsCounter (0),
    itsBuffer  (0),
    itsBufSize (nbuffer)
{}

DH_Example::DH_Example(const DH_Example& that)
  : DataHolder(that),
    itsCounter (0),
    itsBuffer  (0),
    itsBufSize (that.itsBufSize)
{}

DH_Example::~DH_Example()
{}

DataHolder* DH_Example::clone() const
{
  return new DH_Example(*this);
}

void DH_Example::preprocess()
{
  // Add the fields to the data definition.
  addField ("Counter", BlobField<int>(1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  createDataBlock();
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer and initialize the buffer.
  itsBuffer  = getData<BufferType> ("Buffer");
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Example::postprocess()
{
  itsCounter = 0;
  itsBuffer = 0;
}

void DH_Example::dump()
{
  cout << "DH_Example: " << itsBuffer << endl;
}


} // end namespace
