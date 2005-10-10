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
  itsBuffer(0)
{
  //int NrStations      = itsPS.getInt32("PPF.NrStations");
  //int NrPolarizations = itsPS.getInt32("PPF.NrPolarizations");
}

DH_PPF::DH_PPF(const DH_PPF &that)
: DataHolder(that),
  itsBuffer(that.itsBuffer)
{
}

DH_PPF::~DH_PPF()
{
}

DataHolder *DH_PPF::clone() const
{
  return new DH_PPF(*this);
}

void DH_PPF::init()
{
  addField("Buffer", BlobField<i16complex>(1, getBufferSize()));
  createDataBlock();
  itsBuffer = getData<BufferType>("Buffer");
}

void DH_PPF::fillDataPointers()
{
  itsBuffer = getData<BufferType>("Buffer");
}

}
