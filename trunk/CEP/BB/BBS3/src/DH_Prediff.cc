//  DH_Prediff.cc:
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

#include <lofar_config.h>

#include <BBS3/DH_Prediff.h>
#include <BBS3/ParmData.h>
#include <PL/TPersistentObject.h>
#include <Common/KeyValueMap.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

DH_Prediff::DH_Prediff (const string& name)
  : DataHolder    (name, "DH_Prediff", 1),
    itsNResults   (0),
    itsNspids     (0),
    itsNTimes     (0),
    itsNFreq      (0),
    itsDataPtr    (0)
{
  LOG_TRACE_FLOW("DH_Prediff constructor");
  setExtraBlob("Extra", 1);
}

DH_Prediff::DH_Prediff(const DH_Prediff& that)
  : DataHolder    (that),
    itsNResults   (0),
    itsNspids     (0),
    itsNTimes     (0),
    itsNFreq      (0),
    itsDataPtr    (0)
{
  LOG_TRACE_FLOW("DH_Prediff copy constructor");
  setExtraBlob("Extra", 1);
}

DH_Prediff::~DH_Prediff()
{
  LOG_TRACE_FLOW("DH_Prediff destructor");
}

DataHolder* DH_Prediff::clone() const
{
  return new DH_Prediff(*this);
}

void DH_Prediff::preprocess()
{
  LOG_TRACE_FLOW("DH_Prediff preprocess");
  // Add the fields to the data definition.
  addField ("NResults", BlobField<int>(1));
  addField ("NTimes", BlobField<int>(1));
  addField ("NFreq", BlobField<int>(1));
  addField ("Nspids", BlobField<int>(1));
  addField ("DataBuf", BlobField<dcomplex>(1, 0));
}

void DH_Prediff::fillDataPointers()
{
  // Fill in the pointers.
  itsNResults = getData<int> ("NResults");
  itsNspids = getData<int> ("Nspids");
  itsNTimes = getData<int> ("NTimes");
  itsNFreq = getData<int> ("NFreq");
  itsDataPtr = getData<dcomplex> ("DataBuf");
}


void DH_Prediff::postprocess()
{
  LOG_TRACE_FLOW("DH_Prediff postprocess");
  itsNResults = 0;
  itsNspids = 0;
  itsNTimes = 0;
  itsNFreq = 0;
  itsDataPtr = 0;
}

bool DH_Prediff::getParmData(vector<ParmData>& pdata)
{ 
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get vector size.
    int nr;
    bis >> nr;
    pdata.resize(nr);
    for (int i=0; i < nr; i++)
    {
      bis >> pdata[i];
    }
    bis.getEnd();
    return true;
  }
}

void DH_Prediff::setParmData(const vector<ParmData>& pdata)
{
  BlobOStream& bos = createExtraBlob();

  // Put vector length into extra blob
  vector<ParmData>::const_iterator iter;
  int nParms = pdata.size();
  bos << nParms;
  for (iter = pdata.begin(); iter != pdata.end(); iter++)
  {
    bos << *iter;
  }
  bos.putEnd();
}

void DH_Prediff::setData(dcomplex* dataPtr, int size)
{
  vector<uint> sizeVec(1);
  sizeVec[0] = size;
  getDataField("DataBuf").setShape(sizeVec);
  createDataBlock();
  for (int i=0; i<size; i++)
  {
    itsDataPtr[i] = dataPtr[i];     // TBA: Remove this data copy 
  }
}

void DH_Prediff::dump()
{
  cout << "DH_Prediff: " << endl;
  cout << "Number of results = " << getNResults() << endl;
  cout << "Number of spids = " << getNspids() << endl;
  cout << "Number of times = " << getNTimes() << endl;
  cout << "Number of frequencies = " << getNFreq() << endl;

}

void DH_Prediff::clearData()
{
  vector<ParmData> empty;
  setParmData(empty);
}

} // namespace LOFAR
