//  DH_FIR.cc:
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

#include <DH_FIR.h>
//#include <ACC/ParameterSet.h>


namespace LOFAR
{

  DH_FIR::DH_FIR (const string& name, 
		  const short   subband,
		  const ACC::APS::ParameterSet pSet)
    : DataHolder     (name, "DH_FIR"),
      itsBuffer      (0),
      itsFIR         (subband),
      itsNPol        (0),
      itsMatrix      (0),
      itsPS          (pSet)
  {
    itsNStations  = itsPS.getInt32("Input.NRSP");
    itsNTimes     = itsPS.getInt32("Input.NSamplesToDH");
    itsNPol       = itsPS.getInt32("Input.NPolarisations");
  }
  
DH_FIR::DH_FIR(const DH_FIR& that)
  : DataHolder(that),
    itsBuffer(0),
    itsMatrix(0)
{
    itsFIR        = that.itsFIR;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
    itsPS         = that.itsPS;
}

DH_FIR::~DH_FIR()
{
  //  delete [] (char*)(itsDataPacket);
  delete itsMatrix;
  itsBuffer = 0;
}

DataHolder* DH_FIR::clone() const
{
  return new DH_FIR(*this);
}

void DH_FIR::init()
{
  // Determine the number of elements needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", itsNStations));
  vdd.push_back(DimDef("Time", itsNTimes));
  vdd.push_back(DimDef("Polarisation", itsNPol));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);

  itsStationDim = itsMatrix->getDim("Station");
  itsTimeDim    = itsMatrix->getDim("Time");
  itsPolDim     = itsMatrix->getDim("Polarisation");
}

void DH_FIR::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}

void DH_FIR::InitTimeCursor(short station, short pol)
{
  RectMatrix<DH_FIR::BufferType>::cursorType itsTimeCursor;
  itsTimeCursor = itsMatrix->getCursor(station*itsStationDim+pol*itsPolDim);
}

DH_FIR::BufferType DH_FIR::getNextTime()
{
  DH_FIR::BufferType value = itsMatrix->getValue(itsTimeCursor);
  itsMatrix->moveCursor(&itsTimeCursor, itsTimeDim);
  return value;
}

DH_FIR::BufferType DH_FIR::getBufferElement(short /*channel*/, short station, short time, short pol) {
  //  this->InitTimeCursor(station, pol);
  //  itsMatrix->moveCursorN(&itsTimeCursor, itsTimeDim, time);
  return itsMatrix->getValue(itsMatrix->getCursor(station*itsStationDim+pol*itsPolDim+time*itsTimeDim));
}

}
