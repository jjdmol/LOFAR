//  DH_Tester.cc:
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
//
//////////////////////////////////////////////////////////////////////


#include "DH_Tester.h"

namespace LOFAR
{

DH_Tester::DH_Tester (const string& name)
  : DataHolder (name, "DH_Tester"),
    itsCounter (0),
    itsBuffer  (0)
{}

DH_Tester::DH_Tester(const DH_Tester& that)
  : DataHolder(that),
    itsCounter(0),
    itsBuffer(0)
{}

DH_Tester::~DH_Tester()
{}

DataHolder* DH_Tester::clone() const
{
  return new DH_Tester(*this);
}

void DH_Tester::preprocess()
{
  unsigned int bufSize = 10;
  // Add the fields to the data definition.
  addField("Counter", BlobField<int>(1));
  addField("Buffer", BlobField<DataBufferType>(1, //version
                                           bufSize)); //no_elements
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffer.
  for (unsigned int i=0; i<bufSize; i++) {
    itsBuffer[i] = makefcomplex(0,0);
  }
}

void DH_Tester::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer = getData<DataBufferType> ("Buffer");
}

void DH_Tester::postprocess()
{
  itsCounter = 0;
  itsBuffer  = 0;
}


}
