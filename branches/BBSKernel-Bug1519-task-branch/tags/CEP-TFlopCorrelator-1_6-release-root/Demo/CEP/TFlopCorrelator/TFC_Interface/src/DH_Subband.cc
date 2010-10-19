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
  itsBuffer(0),
  itsBufferSize(0)
{
}

DH_Subband::DH_Subband(const DH_Subband &that)
: DataHolder(that),
  itsPS(that.itsPS),
  itsMatrix(0),
  itsBuffer(that.itsBuffer),
  itsBufferSize(that.itsBufferSize)
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
  int norsp = itsPS.getInt32("Data.NStations");
  int ntimes = itsPS.getInt32("Data.NSamplesToIntegrate");
  int nopols = itsPS.getInt32("Data.NPolarisations");
  itsBufferSize = norsp*ntimes*nopols;

  addField("Buffer", BlobField<BufferType>(1, getBufferSize()));
  createDataBlock();

  memset(itsBuffer, 0, sizeof(BufferType) * getBufferSize()); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", norsp));
  vdd.push_back(DimDef("Time", ntimes));
  vdd.push_back(DimDef("Polarisation", nopols));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer((BufferType*)itsBuffer, getBufferSize());
}

void DH_Subband::fillDataPointers()
{
  itsBuffer = getData<BufferType>("Buffer");
}

}
