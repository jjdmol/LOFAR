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
  // this could be done nicer, but it works for now because SyncStamp doesn't contain
  // any pointers
  addField("syncStamp", BlobField<char>(1, sizeof(SyncStamp)));
  createDataBlock();
}

void DH_RSPSync::fillDataPointers() {
  itsSyncStamp = (SyncStamp*)getData<char> ("syncStamp");
  itsSyncStamp->setStamp(0, 0);
}

}
