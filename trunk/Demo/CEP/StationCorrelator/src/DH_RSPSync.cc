//  DH_RSPSync.cc: DataHolder used to synchronise the WH_RSPs
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_RSPSync.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_RSPSync::DH_RSPSync (const string& name)
  : DataHolder    (name, "DH_RSPSync")
{
}

DH_RSPSync::DH_RSPSync(const DH_RSPSync& that)
  : DataHolder(that)
{
}

DH_RSPSync::~DH_RSPSync()
{
}

DataHolder* DH_RSPSync::clone() const
{
  return new DH_RSPSync(*this);
}

void DH_RSPSync::preprocess()
{
  addField("syncStamp", BlobField<syncStamp_t>(1));
  createDataBlock();
}

void DH_RSPSync::fillDataPointers() {
  itsSyncStamp = getData<syncStamp_t> ("syncStamp");
  *itsSyncStamp = 0;
}

}
