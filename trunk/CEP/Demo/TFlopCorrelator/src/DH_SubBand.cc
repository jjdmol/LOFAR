//  DH_SubBand.cc:
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

#include <DH_SubBand.h>
#include <ACC/ParameterSet.h>


namespace LOFAR
{

  DH_SubBand::DH_SubBand (const string& name, 
			  const short   subband)
    : DataHolder     (name, "DH_SubBand"),
      itsBuffer      (0),
      itsSubBand     (subband),
      itsNPol        (0)
  {
    ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
    //ParameterCollection	myPC(myPS);
    itsNFChannels = myPS.getInt("WH_SubBand.freqs");
    itsNStations  = myPS.getInt("WH_SubBand.stations");
    itsNTimes     = myPS.getInt("WH_SubBand.times");
    itsNPol       = myPS.getInt("WH_SubBand.pols");
  }
  
DH_SubBand::DH_SubBand(const DH_SubBand& that)
  : DataHolder(that),
    itsBuffer(0)
{
    itsSubBand    = that.itsSubBand;
    itsNFChannels = that.itsNFChannels;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
}

DH_SubBand::~DH_SubBand()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_SubBand::clone() const
{
  return new DH_SubBand(*this);
}

void DH_SubBand::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 
}

void DH_SubBand::postprocess()
{
  itsBuffer     = 0;
}

void DH_SubBand::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}
}
