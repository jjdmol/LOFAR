//  DH_FixedSize.cc: DataHolder with one dimensional byte array of fixed size
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


#include "3BlockPerf/DH_FixedSize.h"
#include "Common/Debug.h"

namespace LOFAR {

DH_FixedSize::DH_FixedSize (const string& name,
			    unsigned int size)
  : DataHolder (name, "DH_FixedSize")
{
  itsDataPtr = 0;
  itsNoValues = size /sizeof(valueType);
}

DH_FixedSize::DH_FixedSize (const DH_FixedSize& that)
  : DataHolder(that),
    itsNoValues(that.itsNoValues)
{
  itsDataPtr = 0;
}

DH_FixedSize::~DH_FixedSize()
{
}

DataHolder* DH_FixedSize::clone() const
{
  return new DH_FixedSize(*this);
}

void DH_FixedSize::preprocess()
{
  addField ("Data", BlobField<valueType>(1, itsNoValues)); //version, no_elements
  createDataBlock();
}

void DH_FixedSize::fillDataPointers()
{
  itsDataPtr = getData<valueType>("Data");
}

}


