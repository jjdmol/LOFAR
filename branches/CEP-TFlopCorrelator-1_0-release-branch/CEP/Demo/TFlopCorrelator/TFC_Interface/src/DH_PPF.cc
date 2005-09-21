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
#include <APS/ParameterSet.h>


namespace LOFAR
{

  DH_PPF::DH_PPF (const string& name, 
			  const short   subband)
    : DataHolder     (name, "DH_PPF"),
      itsBuffer      (0),
      itsNSamples    (0)
  {
    ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
    itsNSamples = myPS.getInt32("DH_PPF.samples");
    itsNStations  = myPS.getInt32("WH_SubBand.stations");
    itsNTimes     = myPS.getInt32("WH_SubBand.times");
    itsNPol       = myPS.getInt32("WH_SubBand.pols");
    itsNFilters   = myPS.getInt32("WH_SubBand.filters");
  }
  
DH_PPF::DH_PPF(const DH_PPF& that)
  : DataHolder(that),
    itsBuffer(0)
{
  itsNSamples = that.itsNSamples;
}

DH_PPF::~DH_PPF()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_PPF::clone() const
{
  return new DH_PPF(*this);
}

void DH_PPF::init()
{
  // Determine the number of elements needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNPol * itsNTimes; 
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", itsNStations));
  vdd.push_back(DimDef("Polarisation", itsNPol));
  vdd.push_back(DimDef("Time",itsNTimes/itsNFilters)); // the number of FFTs in a dataholder
  vdd.push_back(DimDef("Bank", itsNFilters));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);

  itsStationDim = itsMatrix->getDim("Station");
  itsPolDim     = itsMatrix->getDim("Polarisation");
  itsTimeDim  = itsMatrix->getDim("Time");
  itsBankDim    = itsMatrix->getDim("Bank");
}

void DH_PPF::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}

void DH_PPF::InitTimeCursor(short station, short pol, short time)
{
  RectMatrix<DH_PPF::BufferType>::cursorType itsBankCursor;
  itsBankCursor = itsMatrix->getCursor(station*itsStationDim+pol*itsPolDim+time*itsTimeDim);
}

void DH_PPF::setNextBank(DH_PPF::BufferType &value)
{
  itsMatrix->setValue(itsBankCursor, value);
  itsMatrix->moveCursor(&itsBankCursor, itsBankDim);
  return;
}

}
