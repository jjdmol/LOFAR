//  DH_Example2.cc:
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


#include "DH_Example2.h"
#include <PL/TPersistentObject.h>


namespace LOFAR
{

  DH_Example2::DH_Example2 (const string& name, unsigned int nbuffer,
			    bool useExtra)
: DH_PL      (name, "DH_Example2", 1),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (nbuffer),
  itsPODHEX  (0)                                              //PL
{
  if (useExtra) {
    setExtraBlob ("Extra2", 1);
  }
}

DH_Example2::DH_Example2(const DH_Example2& that)
: DH_PL      (that),
  itsCounter (0),
  itsBuffer  (0),
  itsBufSize (that.itsBufSize),
  itsPODHEX  (0)                                              //PL
{}

DH_Example2::~DH_Example2()
{
  delete itsPODHEX;                                           //PL
}

DataHolder* DH_Example2::clone() const
{
  return new DH_Example2(*this);
}

void DH_Example2::initPO (const string& tableName)           //PL
{                                                            //PL
  itsPODHEX = new PO_DH_EX(*this);                           //PL
  itsPODHEX->tableName (tableName);                          //PL
}                                                            //PL

PL::PersistentObject& DH_Example2::getPO() const             //PL
{                                                            //PL
  return *itsPODHEX;                                         //PL
}                                                            //PL


void DH_Example2::preprocess()
{
  // Add the fields to the data definition.
  addField ("Counter", BlobField<int>(1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffer.
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = makefcomplex(0,0);
  }
}

void DH_Example2::fillDataPointers()
{
  // Fill in the counter pointer.
  itsCounter = getData<int> ("Counter");
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");
}

void DH_Example2::postprocess()
{
  itsCounter = 0;
  itsBuffer = 0;
}


namespace PL {                                               //PL

void DBRep<DH_Example2>::bindCols (dtl::BoundIOs& cols)      //PL
{                                                            //PL
  DBRep<DH_PL>::bindCols (cols);                             //PL
  cols["COUNTER"] == itsCounter;                             //PL
}                                                            //PL

void DBRep<DH_Example2>::toDBRep (const DH_Example2& obj)    //PL
{                                                            //PL
  DBRep<DH_PL>::toDBRep (obj);                               //PL
  itsCounter = obj.getCounter();                             //PL
}                                                            //PL

//# Force the instantiation of the template.                 //PL
template class TPersistentObject<DH_Example2>;               //PL

}  // end namespace PL                                       //PL 


} // end namespace LOFAR
