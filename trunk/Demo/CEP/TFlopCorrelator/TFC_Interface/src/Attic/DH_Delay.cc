//#  DH_Delay.cc: dataholder to hold the delay information to perform
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

#include <DH_Delay.h>

namespace LOFAR
{

DH_Delay::DH_Delay (const string& name)
    : DataHolder     (name, "DH_Delay")
{
  
}
  
DH_Delay::DH_Delay(const DH_Delay& that)
  : DataHolder (that)
{   
}

DH_Delay::~DH_Delay()
{
}

DataHolder* DH_Delay::clone() const
{
  return new DH_Delay(*this);
}

void DH_Delay::init()
{
  // add the fields to the data definition
  addField ("Delay", BlobField<int>(1, 1));
  addField ("SeqId", BlobField<int>(1, 1));
  addField ("BlockId", BlobField<int>(1, 1));
  
  // create the data blob
  createDataBlock();
}

void DH_Delay::fillDataPointers() 
{
  itsDelayPtr = getData<int> ("Delay");
  itsSeqIdPtr = getData<int> ("SeqId");
  itsBlockIdPtr = getData<int> ("BlockId");
}

}  // end namespace
