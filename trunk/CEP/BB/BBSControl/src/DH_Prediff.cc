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

#include <BBSControl/DH_Prediff.h>
#include <BBSControl/BlobAipsIO.h>
#include <BBS/ParmData.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <casa/IO/AipsIO.h>
#include <scimath/Fitting/LSQFit.h>

#include <sstream>
#include <unistd.h> 

using namespace casa;

namespace LOFAR
{

DH_Prediff::DH_Prediff (const string& name)
  : DataHolder    (name, "DH_Prediff", 1),
    itsDataSize   (0),
    itsDataBuffer (0),
    itsStartFreq  (0),
    itsEndFreq    (0),
    itsStartTime  (0),
    itsEndTime    (0)
{
  LOG_TRACE_FLOW("DH_Prediff constructor");
  setExtraBlob("Extra", 1);
}

DH_Prediff::DH_Prediff(const DH_Prediff& that)
  : DataHolder    (that),
    itsDataSize   (0),
    itsDataBuffer (0),
    itsStartFreq  (0),
    itsEndFreq    (0),
    itsStartTime  (0),
    itsEndTime    (0)
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

void DH_Prediff::init()
{
  LOG_TRACE_FLOW("DH_Prediff preprocess");
  // Add the fields to the data definition.
  addField ("DataSize", BlobField<unsigned int>(1));
  addField ("StartFreq", BlobField<double>(1));
  addField ("EndFreq", BlobField<double>(1));
  addField ("StartTime", BlobField<double>(1));
  addField ("EndTime", BlobField<double>(1));
  addField ("DataBuf", BlobField<double>(1, 0u));

  // Create the data blob (which calls fillPointers).
  createDataBlock();

  *itsDataSize = 0;
  *itsStartFreq = 0;
  *itsEndFreq = 0;
  *itsStartTime = 0;
  *itsEndTime = 0;
}

void DH_Prediff::fillDataPointers()
{
  // Fill in the pointers.
  itsDataSize = getData<unsigned int> ("DataSize");
  itsDataBuffer = getData<double> ("DataBuf");
  itsStartFreq = getData<double> ("StartFreq");
  itsEndFreq = getData<double> ("EndFreq");
  itsStartTime = getData<double> ("StartTime");
  itsEndTime = getData<double> ("EndTime");
}

bool DH_Prediff::getParmData (ParmDataInfo& pdata)
{ 
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob (found, version);
  if (found) {
    bis >> pdata;
  }
  return found;
}

void DH_Prediff::setParmData (const ParmDataInfo& pdata)
{
  BlobOStream& bos = createExtraBlob();
  bos << pdata;
}

void DH_Prediff::setFitters (const vector<LSQFit>& fitters)
{
  BlobOStream& bos = createExtraBlob();
  BlobAipsIO bio(bos);
  casa::AipsIO aio(&bio);
  aio.putstart ("fitters", 1);
  aio << fitters.size();
  for (uint i=0; i<fitters.size(); ++i) {
    fitters[i].toAipsIO (aio);
  }
  aio.putend();
}

bool DH_Prediff::getFitters (vector<LSQFit>& fitters)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob (found, version);
  if (found) {
    BlobAipsIO bio(bis);
    casa::AipsIO aio(&bio);
    aio.getstart ("fitters");
    uint sz;
    aio >> sz;
    fitters.resize (sz);
    for (uint i=0; i<sz; ++i) {
      fitters[i].fromAipsIO (aio);
    }
    aio.getend();
  }
  return found;
}

void DH_Prediff::setBufferSize (int size)
{
  vector<uint32> shp(1);
  shp[0] = size;
  getDataField("DataBuf").setShape(shp);
  createDataBlock();
}

int DH_Prediff::getBufferSize()
{
  return getDataField("DataBuf").getShape()[0];
}

void DH_Prediff::setDomain(double fStart, double fEnd, double tStart, double tEnd)
{
  *itsStartFreq = fStart;
  *itsEndFreq = fEnd;
  *itsStartTime = tStart;
  *itsEndTime = tEnd;
}

void DH_Prediff::dump() const
{
  cout << "DH_Prediff: " << endl;
  cout << "Parm data : " << endl;
  cout << "Start frequency = " << getStartFreq() << endl;
  cout << "End frequency = " << getEndFreq() << endl;
  cout << "Start time = " << getStartTime() << endl;
  cout << "End time = " << getEndTime() << endl;
  ParmDataInfo pData;
  if (const_cast<DH_Prediff*>(this)->getParmData(pData)) {
    cout << pData << endl;
  }
}

void DH_Prediff::clearData()
{
  setParmData (ParmDataInfo());
}

} // namespace LOFAR
