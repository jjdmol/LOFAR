//#  DH_Sync.cc: dataholder to hold the delay information to perform
//#              station synchronization
//#
//#  Copyright (C) 2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//#
//#
//////////////////////////////////////////////////////////////////////


#include <lofar_config.h>

#include <DH_Sync.h>

namespace LOFAR
{

DH_Sync::DH_Sync (const string& name)
    : DataHolder     (name, "DH_Sync")
{
  
}
  
DH_Sync::DH_Sync(const DH_Sync& that)
  : DataHolder (that)
{   
}

DH_Sync::~DH_Sync()
{
}

DataHolder* DH_Sync::clone() const
{
  return new DH_Sync(*this);
}

void DH_Sync::init()
{
  // add the fields to the data definition
  addField ("Delay", BlobField<int>(1, 1));
  addField ("SeqId", BlobField<int>(1, 1));
  addField ("BlockId", BlobField<int>(1, 1));
  
  // create the data blob
  createDataBlock();
}

void DH_Sync::fillDataPointers() 
{
  itsDelayPtr = getData<int> ("Delay");
  itsSeqIdPtr = getData<int> ("SeqId");
  itsBlockIdPtr = getData<int> ("BlockId");
}

}  // end namespace
