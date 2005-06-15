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

  DH_PPF::DH_PPF (const string& name, 
			  const short   subband)
    : DataHolder     (name, "DH_PPF"),
      itsBuffer      (0),
      itsSubBand     (subband),
      itsNPol        (0)
  {
//     ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
//     //ParameterCollection	myPC(myPS);
//     itsNFChannels = myPS.getInt("WH_SubBand.freqs");
//     itsNStations  = myPS.getInt("WH_SubBand.stations");
//     itsNTimes     = myPS.getInt("WH_SubBand.times");
//     itsNPol       = myPS.getInt("WH_SubBand.pols");
  }
  
DH_PPF::DH_PPF(const DH_PPF& that)
  : DataHolder(that),
    itsBuffer(0)
{
    itsSubBand    = that.itsSubBand;
    itsNFChannels = that.itsNFChannels;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
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
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 
}

void DH_PPF::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}
}
