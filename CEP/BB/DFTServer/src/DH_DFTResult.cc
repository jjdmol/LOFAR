//  DH_DFTResult.cc:
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


#include <PSS3/DH_DFTResult.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

DH_DFTResult::DH_DFTResult()
: DataHolder ("", "DH_DFTResult"),
  itsValues  (0)
{}

DH_DFTResult::DH_DFTResult(const DH_DFTResult& that)
: DataHolder (that),
  itsValues  (0)
{}

DH_DFTResult::~DH_DFTResult()
{}

DataHolder* DH_DFTResult::clone() const
{
  return new DH_DFTResult(*this);
}

void DH_DFTResult::preprocess()
{
  // Add the fields to the data definition.
  // The dimensionality of Data is variable.
  // Its axes are nfreq,ntime,nbaseline.
  // Its data is complex<double> represented by 2 doubles.
  addField ("Values", BlobField<double>(1, 0u, 0u, 0u, 2u));
}

void DH_DFTResult::set (int nFreq, int nTime, int nBaseline)
{
  std::vector<uint32> shape(4);
  shape[0] = nFreq;
  shape[1] = nTime;
  shape[2] = nBaseline;
  shape[3] = 2;
  getDataField("Values").setShape (shape);
  // Create the data blob (which calls fillDataPointers).
  createDataBlock();
}

void DH_DFTResult::fillDataPointers()
{
  // Fill in the data pointer.
  itsValues = getData<double> ("Values");
  const std::vector<uint32>& shape = getDataField("Values").getShape();
  ASSERT (shape.size() == 4);
  itsNFreq = shape[0];
  itsNTime = shape[1];
  itsNBaseline = shape[2];
  ASSERT (shape[3] == 2);
}


void DH_DFTResult::postprocess()
{
  itsValues = 0;
}

} // end namespace
