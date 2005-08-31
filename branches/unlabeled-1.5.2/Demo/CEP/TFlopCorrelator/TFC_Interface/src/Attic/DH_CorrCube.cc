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
#include <lofar_config.h>

#include <DH_CorrCube.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{

  DH_CorrCube::DH_CorrCube (const string& name, short subband)
    : DataHolder     (name, "DH_CorrCube"),
      itsBuffer      (0),
      itsSubBand     (subband),
      itsNPol        (0)
  {
    ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
    //ParameterCollection	myPC(myPS);
    itsNFChannels = myPS.getInt32("DH_CorrCube.freqs");
    itsNStations  = myPS.getInt32("DH_CorrCube.stations");
    itsNTimes     = myPS.getInt32("DH_CorrCube.times");
    itsNPol       = myPS.getInt32("Input.NPolarisations");
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

void DH_CorrCube::init()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Flag", BlobField<int>(1, 1));
  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();  // calls fillDataPointers
  // itsBuffer = getData<BufferType> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 
}

void DH_CorrCube::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}

void DH_CorrCube::print()
{
  int lines = 0;
  for (int channel = 0; channel < itsNFChannels; channel ++)
    for (int station = 0; station < itsNStations; station ++)
      for (int time = 0; time < itsNTimes; time ++)
	for (int polarization = 0; polarization < itsNPol; polarization ++) {
	  if (++ lines >= 100)
	    std::cerr << channel << ' ' << station << ' ' << time << ' ' << polarization << ": " << *getBufferElement(channel, station, time, polarization) << '\n';
	  if (lines == 110) return;
	}
}

}
