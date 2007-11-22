//  DH_ExampleExtra3.cc:
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

#include "DH_ExampleExtra3.h"
#include <TransportPostgres/TH_DB.h>

namespace LOFAR
{

DH_ExampleExtra3::DH_ExampleExtra3 (const string& name, unsigned int initialNelements)
: DH_DB     (name, "DH_ExampleExtra3", 1),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (initialNelements)
{
  setExtraBlob ("Extra", 1);
}

DH_ExampleExtra3::DH_ExampleExtra3(const DH_ExampleExtra3& that)
: DH_DB      (that),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize)
{}

DH_ExampleExtra3::~DH_ExampleExtra3()
{}

DataHolder* DH_ExampleExtra3::clone() const
{
  return new DH_ExampleExtra3(*this);
}

void DH_ExampleExtra3::init()
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

void DH_ExampleExtra3::setBufferSize (unsigned int nelements)
{
  itsBufSize = nelements;
  init();
}

void DH_ExampleExtra3::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

DH_ExampleExtra3::BufferType& DH_ExampleExtra3::getBufferElement(unsigned int element)
{
  ASSERT(element < itsBufSize); 
  return itsBuffer[element];
}

string DH_ExampleExtra3::createInsertStatement(TH_DB* th)
{
  ostringstream q;
  q << "INSERT INTO exampledb VALUES ('";
  th->addDBBlob(this, q);
  q << "', "
    << getCounter() << ")";
  return q.str();
}

string DH_ExampleExtra3::createUpdateStatement(TH_DB* th)
{
  ostringstream q;
  q << "UPDATE exampledb SET data='";
  th->addDBBlob(this, q);
  q << "' WHERE counter=" << getCounter();
  return q.str();
}

} // end namespace
