//  DH_CorrCube.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_CorrCube.h>
#include <ACC/ParameterSet.h>

namespace LOFAR
{

  DH_CorrCube::DH_CorrCube (const string& name, short subband)
    : DataHolder     (name, "DH_CorrCube"),
      itsBuffer      (0),
      itsSubBand     (subband),
      itsNPol        (2)
  {
    ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
    //ParameterCollection	myPC(myPS);
    itsNFChannels = myPS.getInt("DH_CorrCube.freqs");
    itsNStations  = myPS.getInt("DH_CorrCube.stations");
    itsNTimes     = myPS.getInt("DH_CorrCube.times");
  }
  
DH_CorrCube::DH_CorrCube(const DH_CorrCube& that)
  : DataHolder(that),
    itsBuffer(0)
{
    itsSubBand    = that.itsSubBand;
    itsNFChannels = that.itsNFChannels;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
}

DH_CorrCube::~DH_CorrCube()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_CorrCube::clone() const
{
  return new DH_CorrCube(*this);
}

void DH_CorrCube::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<fcomplex>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  // itsBuffer = getData<fcomplex> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(fcomplex)); 
}

void DH_CorrCube::postprocess()
{
  itsBuffer     = 0;
}

void DH_CorrCube::fillDataPointers() {
  itsBuffer = getData<fcomplex> ("Buffer");
}
}
