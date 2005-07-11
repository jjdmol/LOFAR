//  DH_StationSB.cc:  DataHolder containing ~1s of data for one station, one subband
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

#include <TFC_Interface/DH_StationSB.h>

namespace LOFAR
{

  DH_StationSB::DH_StationSB (const string& name, 
			      const short   subband)
    : DataHolder     (name, "DH_StationSB"),
      itsBuffer      (0),
      itsSubband     (subband),
      itsMatrix      (0)
  {
    itsNFChannels = 5;
    itsNTimes     = 1000;
    itsNPol       = 2;
  }
  
DH_StationSB::DH_StationSB(const DH_StationSB& that)
  : DataHolder(that),
    itsBuffer(0),
    itsMatrix(0)
{
    itsSubband    = that.itsSubband;
    itsNFChannels = that.itsNFChannels;
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
}

DH_StationSB::~DH_StationSB()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_StationSB::clone() const
{
  return new DH_StationSB(*this);
}

void DH_StationSB::init()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("FreqChannel", itsNFChannels));
  vdd.push_back(DimDef("Time", itsNTimes));
  vdd.push_back(DimDef("Polarisation", itsNPol));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);
}

void DH_StationSB::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}
}
