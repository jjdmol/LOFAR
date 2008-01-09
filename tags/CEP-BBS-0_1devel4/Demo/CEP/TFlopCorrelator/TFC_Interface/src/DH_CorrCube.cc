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

  DH_CorrCube::DH_CorrCube (const string& name, short subband, const ACC::APS::ParameterSet& ps)
    : DataHolder     (name, "DH_CorrCube"),
      itsBuffer      (0),
      itsSubBand     (subband)//,
      //itsNPol        (0)
  {
#if 0
    //ParameterCollection	myPC(ps);
    itsNFChannels = ps.getInt32("DH_CorrCube.freqs");
    itsNStations  = ps.getInt32("DH_CorrCube.stations");
    itsNTimes     = ps.getInt32("DH_CorrCube.times");
    itsNPol       = ps.getInt32("Input.NPolarisations");
#endif
  }
  
DH_CorrCube::DH_CorrCube(const DH_CorrCube& that)
  : DataHolder(that),
    itsBuffer(0)
{
    itsSubBand    = that.itsSubBand;
#if 0
    itsNFChannels = that.itsNFChannels;
    itsNStations  = that.itsNStations; 
    itsNTimes     = that.itsNTimes;
    itsNPol       = that.itsNPol;
#endif
}

DH_CorrCube::~DH_CorrCube()
{
}

DataHolder* DH_CorrCube::clone() const
{
  return new DH_CorrCube(*this);
}

void DH_CorrCube::init()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  //itsBufSize = itsNStations * itsNFChannels * itsNTimes * itsNPol;
  
  addField ("Buffer", BlobField<fcomplex>(1, getBufSize()), 32);
  addField ("Flag", BlobField<int>(1, 1));
  
  createDataBlock();  // calls fillDataPointers
  // itsBuffer = getData<BufferType> ("Buffer");
  //memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 
  memset(itsBuffer, 0, sizeof(BufferType));
}

void DH_CorrCube::fillDataPointers() {
  itsBuffer = (BufferType*)getData<fcomplex> ("Buffer");
}

void DH_CorrCube::print()
{
  unsigned long long chksum = 0;

  int lines = 0;
  for (int station = 0; station < NR_STATIONS; station ++) {
    for (int polarization = 0; polarization < NR_POLARIZATIONS; polarization ++)
      for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time ++) {
	fcomplex chksum4 = makefcomplex(0,0);

	for (int channel = 0; channel < NR_SUB_CHANNELS; channel ++) {
	  chksum4 += (*itsBuffer)[station][polarization][time][channel];
	}

	chksum ^= * (unsigned long long *) &chksum4;
      }
  }
  std::cerr << chksum << '\n';
}

}
