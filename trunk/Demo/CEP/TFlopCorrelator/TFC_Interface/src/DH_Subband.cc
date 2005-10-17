//  DH_Subband.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <lofar_config.h>

#include <DH_Subband.h>
//#include <ACC/ParameterSet.h>


namespace LOFAR
{

DH_Subband::DH_Subband(const string &name, 
	       const short subBandId,
	       const ACC::APS::ParameterSet pSet)
: DataHolder(name, "DH_Subband"),
  itsPS(pSet),
  itsMatrix(0),
  itsBuffer(0)
{
}

DH_Subband::DH_Subband(const DH_Subband &that)
: DataHolder(that),
  itsPS(that.itsPS),
  itsMatrix(0),
  itsBuffer(that.itsBuffer)
{
}

DH_Subband::~DH_Subband()
{
  delete itsMatrix;
  itsBuffer = 0;
}

DataHolder *DH_Subband::clone() const
{
  return new DH_Subband(*this);
}

void DH_Subband::init()
{
  addField("Buffer", BlobField<BufferType>(1, getBufferSize()));
  createDataBlock();

  memset(itsBuffer, 0, sizeof(BufferType) * getBufferSize()); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", pset.getInt32("Input.NRSP")));
  vdd.push_back(DimDef("Time", pset.getInt32("Input.NSamplesToDH")));
  vdd.push_back(DimDef("Polarisation", pset.getInt32("Input.NPolarisations")));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer((BufferType*)itsBuffer, getBufferSize());
}

void DH_Subband::fillDataPointers()
{
  itsBuffer = getData<BufferType>("Buffer");
}

}
