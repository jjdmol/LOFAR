//  DH_DFTRequest.cc:
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


#include <DH_DFTRequest.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

DH_DFTRequest::DH_DFTRequest()
: DataHolder ("", "DH_DFTRequest"),
  itsUVW     (0)
{}

DH_DFTRequest::DH_DFTRequest(const DH_DFTRequest& that)
: DataHolder (that),
  itsUVW     (0)
{}

DH_DFTRequest::~DH_DFTRequest()
{}

DataHolder* DH_DFTRequest::clone() const
{
  return new DH_DFTRequest(*this);
}

void DH_DFTRequest::preprocess()
{
  // Add the fields to the data definition.
  // The dimensionality of UVW is variable.
  // Its axes are ntime and nantenna.
  addField ("UVW", BlobField<double>(1, 0u, 0u));
  addField ("StartFreq", BlobField<double>(1));
  addField ("StepFreq", BlobField<double>(1));
  addField ("NFreq", BlobField<int32>(1));
  addField ("NTime", BlobField<int32>(1));
  addField ("StartTime", BlobField<double>(1));
  addField ("StepTime", BlobField<double>(1));
  addField ("L", BlobField<double>(1));
  addField ("M", BlobField<double>(1));
  addField ("N", BlobField<double>(1));
  // The Ant nrs tell the antenna pairs froming the baselines.
  addField ("Ant", BlobField<int32> (1, 0u));
  addField ("Ant1", BlobField<int32> (1, 0u));
  addField ("Ant2", BlobField<int32> (1, 0u));
  createDataBlock();
}

void DH_DFTRequest::set (double startFreq, double stepFreq, int nFreq,
		    double startTime, double stepTime, int nTime,
		    int nAnt, int nBaseline)
{
  std::vector<uint32> shape(2);
  shape[0] = nTime;
  shape[1] = nAnt;
  getDataField("UVW").setShape (shape);
  getDataField("Ant").setShape (std::vector<uint32>(1,nAnt));
  getDataField("Ant1").setShape (std::vector<uint32>(1,nBaseline));
  getDataField("Ant2").setShape (std::vector<uint32>(1,nBaseline));
  // Create the data blob (which calls fillDataPointers).
  createDataBlock();
  *itsStartFreq = startFreq;
  *itsStepFreq  = stepFreq;
  *itsNFreq     = nFreq;
  *itsStartTime = startTime;
  *itsStepTime  = stepTime;
  *itsNTime     = nTime;
}

void DH_DFTRequest::fillDataPointers()
{
  // Fill in the data pointers.
  itsUVW = getData<double> ("UVW");
  itsNFreq     = getData<int32> ("NFreq");
  itsStartFreq = getData<double> ("StartFreq");
  itsStepFreq  = getData<double> ("StepFreq");
  itsStartTime = getData<double> ("StartTime");
  itsStepTime  = getData<double> ("StepTime");
  itsNTime     = getData<int32> ("NTime");
  itsL         = getData<double> ("L");
  itsM         = getData<double> ("M");
  itsN         = getData<double> ("N");
  itsAnt       = getData<int32>  ("Ant");
  itsNAnt      = getDataField("Ant").getNelem();
  itsAnt1      = getData<int32>  ("Ant1");
  itsAnt2      = getData<int32>  ("Ant2");
  itsNBaseline = getDataField("Ant1").getNelem();
  ASSERT (itsNBaseline == getDataField("Ant2").getNelem());
  const std::vector<uint32>& shape = getDataField("UVW").getShape();
  ASSERT (shape.size() == 2);
  ASSERT (*itsNTime == shape[0]);
  ASSERT (itsNAnt == shape[1]);
}


void DH_DFTRequest::postprocess()
{
  itsUVW = 0;
}

} // end namespace
