//# DH_Socket.cc:
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include "DH_Socket.h"

namespace LOFAR
{

DH_Socket::DH_Socket (const string& name, uint32 initialNrElements)
: DataHolder (name, "DH_Socket", 1),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (initialNrElements)
{
}

DH_Socket::DH_Socket(const DH_Socket& that)
: DataHolder (that),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_Socket::~DH_Socket()
{}

DataHolder* DH_Socket::clone() const
{
  return new DH_Socket(*this);
}

void DH_Socket::init()
{
  // Initialize the fieldset.
  initDataFields();
  // Add the fields to the data definition.
  addField ("Counter", BlobField<int32>(1));
  addField ("Buffer", BlobField<BufferType>(1, //version 
					    itsBufSize)); //no_elements
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffer.
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Socket::setBufferSize (uint32 nelements)
{
  itsBufSize = nelements;
  init();
}

void DH_Socket::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

} // end namespace
