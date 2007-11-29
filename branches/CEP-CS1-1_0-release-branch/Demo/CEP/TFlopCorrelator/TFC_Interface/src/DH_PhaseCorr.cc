//#  DH_PhaseCorr.cc: dataholder to hold the delay information to perform
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

#include <TFC_Interface/DH_PhaseCorr.h>

namespace LOFAR
{

DH_PhaseCorr::DH_PhaseCorr (const string& name, int nrChannels)
  : DataHolder     (name, "DH_PhaseCorr"),
    itsNrChannels      (nrChannels)
{
  
}
  
DH_PhaseCorr::DH_PhaseCorr(const DH_PhaseCorr& that)
  : DataHolder (that),
    itsNrChannels  (that.itsNrChannels)
{   
}

DH_PhaseCorr::~DH_PhaseCorr()
{
}

DataHolder* DH_PhaseCorr::clone() const
{
  return new DH_PhaseCorr(*this);
}

void DH_PhaseCorr::init()
{
  // add the fields to the data definition
  addField ("PhaseCorrs", BlobField<fcomplex>(1, itsNrChannels), 32);
  
  // create the data blob
  createDataBlock();

  for (int i=0; i<itsNrChannels; i++) {
    itsCorrPtr[i] = makefcomplex(0,0);
  }

}

void DH_PhaseCorr::fillDataPointers() 
{
  itsCorrPtr = getData<fcomplex> ("PhaseCorrs");
}

const fcomplex DH_PhaseCorr::getPhaseCorr(int index) const
{ 
  ASSERTSTR((index < itsNrChannels) && (index >= 0), "index is not within range");
  return itsCorrPtr[index]; 
}

void DH_PhaseCorr::setPhaseCorr(int index, fcomplex value)
{ 
  ASSERTSTR((index < itsNrChannels) && (index >= 0), "index is not within range");
  itsCorrPtr[index] = value;
}

}  // end namespace
