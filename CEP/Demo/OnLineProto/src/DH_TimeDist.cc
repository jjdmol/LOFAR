//  DH_TimeDist.cc:
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
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_TimeDist.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

  DH_TimeDist::DH_TimeDist (const string& name)
    : DataHolder            (name, "DH_TimeDist")
{
}

DH_TimeDist::DH_TimeDist(const DH_TimeDist& that)
  : DataHolder     (that)
{
}

DH_TimeDist::~DH_TimeDist()
{
}

DataHolder* DH_TimeDist::clone() const
{
  return new DH_TimeDist(*this);
}

void DH_TimeDist::preprocess()
{

  addField("Time",BlobField< int >(1,1));
  addField("DeltaTime",BlobField< int >(1,1));
 // Create the data blob (which calls fillPointers).
  createDataBlock();

}

void DH_TimeDist::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsTimeptr         = getData<int> ("Time");
  itsDeltaTimeptr    = getData<int> ("DeltaTime");
}


void DH_TimeDist::postprocess()
{
  itsTimeptr = 0;
  itsDeltaTimeptr = 0;
}

}
