//# DH_PPF.h: SubBand DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_PPF_H
#define TFLOPCORRELATOR_DH_PPF_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_PPF: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_PPF (const string& name,
		       const short   subband); 


  DH_PPF(const DH_PPF&);

  virtual ~DH_PPF();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  // todo: get pointer to array that can serve as input for the fft
  //BufferType* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  BufferType* setBufferElement(int channel, int station, int sample, int polarisation);

private:
  /// Forbid assignment.
  DH_PPF& operator= (const DH_PPF&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNFChannels;
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  void fillDataPointers();
};

//exact addressing to be defined 
#define SBADDRESS_STATION(freq, station)        itsNPol*itsNTimes*itsNStations*(station) 
#define SBADDRESS_TIME(freq, station, time)     SBADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define SBADDRESS_POL(freq, station, time, pol) SBADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
 
 
/*  inline void DH_PPF::setBufferElement(int channel, */
/* 					  int sample, */
/* 					  int station, */
/* 					  int polarisation, */
/* 					  DH_PPF::BufferType* valueptr) { */
/*    *(itsBuffer + SBADDRESS_POL(channel, station, sample, pol)) = *valueptr; */
/*  } */
 
}
#endif 
