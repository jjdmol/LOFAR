//  DH_VarSize.cc: DataHolder with one dimensional byte array that 
//                  can grow its size (e.g. for performance measurements)
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


#include "P2Perf/DH_VarSize.h"
#include "Common/Debug.h"
#include "Common/BlobException.h"

using namespace LOFAR;

DH_VarSize::DH_VarSize (const string& name,
			unsigned int initialSize,
                        unsigned int maxSize)
  : DataHolder (name, "DH_VarSize"),
    itsMaxSpoofedDataSize(maxSize),
    itsSpoofedDataSize(initialSize),
    itsOverhead(40)
{
}

DH_VarSize::DH_VarSize (const DH_VarSize& that)
  : DataHolder(that),
    itsMaxSpoofedDataSize(that.itsMaxSpoofedDataSize),
    itsSpoofedDataSize(that.itsSpoofedDataSize),
    itsOverhead(40)
{
}

DH_VarSize::~DH_VarSize()
{
}

DataHolder* DH_VarSize::clone() const
{
  return new DH_VarSize(*this);
}

void DH_VarSize::preprocess()
{
  addField ("dummyData", BlobField<char>(1, itsMaxSpoofedDataSize)); //version, no_elements
  createDataBlock();
  
  // the real data size will probable be bigger due to some overhead
  itsOverhead = DataHolder::getDataSize() - itsMaxSpoofedDataSize;
}

void DH_VarSize::setSpoofedDataSize(int dataSize)
{
  if (dataSize < itsMaxSpoofedDataSize)
  {
    itsSpoofedDataSize = dataSize;
  }
  else
  {
    itsSpoofedDataSize = itsMaxSpoofedDataSize;
  }
}



