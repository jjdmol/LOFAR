//  DH_PPF.cc:
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

#include <DH_PPF.h>
//#include <ACC/ParameterSet.h>


namespace LOFAR
{

DH_PPF::DH_PPF(const string &name, 
	       const short subBandId,
	       const ACC::APS::ParameterSet pSet)
: DataHolder(name, "DH_PPF"),
  itsPS(pSet),
  itsMatrix(0),
  itsBuffer(0)
{
}

DH_PPF::DH_PPF(const DH_PPF &that)
: DataHolder(that),
  itsPS(that.itsPS),
  itsMatrix(0),
  itsBuffer(that.itsBuffer)
{
}

DH_PPF::~DH_PPF()
{
  delete itsMatrix;
  itsBuffer = 0;
}

DataHolder *DH_PPF::clone() const
{
  return new DH_PPF(*this);
}

void DH_PPF::init()
{
  addField("Buffer", BlobField<BufferElementType>(1, getBufferSize()));
  createDataBlock();

  //memset(itsBuffer, 0, sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", MAX_STATIONS_PER_PPF));
  vdd.push_back(DimDef("Time", NR_STATION_SAMPLES));
  vdd.push_back(DimDef("Polarisation", NR_POLARIZATIONS));
  
  itsMatrix = new RectMatrix<BufferElementType> (vdd);
  itsMatrix->setBuffer((BufferElementType*)itsBuffer, getBufferSize());
}

void DH_PPF::fillDataPointers()
{
  itsBuffer = (BufferType*) getData<BufferElementType>("Buffer");
}

}
