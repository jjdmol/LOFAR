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
			  const short   subband)
    : DataHolder     (name, "DH_FIR"),
      itsBuffer      (0),
      itsFIR     (subband),
      itsNPol        (0),
      itsMatrix      (0)
  {
//     ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
//     //ParameterCollection	myPC(myPS);
//     itsNFChannels = myPS.getInt("WH_FIR.freqs");
//     itsNStations  = myPS.getInt("WH_FIR.stations");
//     itsNTimes     = myPS.getInt("WH_FIR.times");
//     itsNPol       = myPS.getInt("WH_FIR.pols");
    itsNFChannels = 2;
    itsNStations  = 3;
    itsNTimes     = 4;
    itsNPol       = 5;
  }
  
DH_FIR::DH_FIR(const DH_FIR& that)
  : DataHolder(that),
    itsBuffer(0),
    itsMatrix(0)
{
    itsFIR    = that.itsFIR;
    itsNFChannels = that.itsNFChannels;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
}

DH_FIR::~DH_FIR()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_FIR::clone() const
{
  return new DH_FIR(*this);
}

void DH_FIR::init()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Station", itsNStations));
  vdd.push_back(DimDef("FreqChannel", itsNFChannels));
  vdd.push_back(DimDef("Time", itsNTimes));
  vdd.push_back(DimDef("Polarisation", itsNPol));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);
}

void DH_FIR::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}
}
