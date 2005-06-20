//  DH_RSPInputSync.cc: DataHolder used to synchronize incoming RSP data
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


#include <DH_RSPInputSync.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_RSPInputSync::DH_RSPInputSync (const string& name)
  : DataHolder    (name, "DH_RSPInputSync")
{
}

DH_RSPInputSync::DH_RSPInputSync(const DH_RSPInputSync& that)
  : DataHolder(that)
{
}

DH_RSPInputSync::~DH_RSPInputSync()
{
}

DataHolder* DH_RSPInputSync::clone() const
{
  return new DH_RSPInputSync(*this);
}

void DH_RSPInputSync::init()
{
  // this could be done nicer, but it works for now because SyncStamp doesn't contain
  // any pointers
  addField("RSPsyncStamp", BlobField<char>(1, sizeof(timestamp_t)));
  createDataBlock();
}

void DH_RSPInputSync::fillDataPointers() {
  itsSyncStamp = (timestamp_t*)getData<char> ("RSPsyncStamp");
  itsSyncStamp->setStamp(0, 0);
}

}
