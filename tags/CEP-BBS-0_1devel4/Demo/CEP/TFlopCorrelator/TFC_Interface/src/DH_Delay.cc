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

#include <TFC_Interface/DH_Delay.h>

namespace LOFAR
{

DH_Delay::DH_Delay (const string& name, int nrRSPs)
  : DataHolder     (name, "DH_Delay"),
    itsNrRSPs      (nrRSPs)
{
  
}
  
DH_Delay::DH_Delay(const DH_Delay& that)
  : DataHolder (that),
    itsNrRSPs  (that.itsNrRSPs)
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
  addField ("Delay", BlobField<int>(1, itsNrRSPs));
  
  // create the data blob
  createDataBlock();

  for (int i=0; i<itsNrRSPs; i++) {
    itsDelayPtr[i] = 0;
  }

}

void DH_Delay::fillDataPointers() 
{
  itsDelayPtr = getData<int> ("Delay");
}

const int DH_Delay::getDelay(int index) const
{ 
  ASSERTSTR((index < itsNrRSPs) && (index >= 0), "index is not within range");
  return itsDelayPtr[index]; 
}

void DH_Delay::setDelay(int index, int value)
{ 
  ASSERTSTR((index < itsNrRSPs) && (index >= 0), "index is not within range");
  itsDelayPtr[index] = value;
}

}  // end namespace
