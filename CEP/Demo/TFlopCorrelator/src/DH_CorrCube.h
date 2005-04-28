//# DH_CorrCube.h: CorrCube DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_CORRCUBE_H
#define TFLOPCORRELATOR_DH_CORRCUBE_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_CorrCube: public DataHolder
{
public:

  explicit DH_CorrCube (const string& name, short subband); 


  DH_CorrCube(const DH_CorrCube&);

  virtual ~DH_CorrCube();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  fcomplex* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const fcomplex* getBuffer() const;

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop
  fcomplex* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  fcomplex* getBufferElement(int channel, int station, int sample, int polarisation);
  void setBufferElement(int channel, int station, int sample, int polarisation, fcomplex* value); 

  const unsigned int getBufSize() const;

private:
  /// Forbid assignment.
  DH_CorrCube& operator= (const DH_CorrCube&);

  fcomplex*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNFChannels;     // #frequency channels per buffer
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  void fillDataPointers();
};


#define CCADDRESS_FREQ(freq) itsNPol*itsNTimes*itsNStations*itsNFChannels*(freq)
#define CCADDRESS_STATION(freq, station) CCADDRESS_FREQ((freq)) +  itsNPol*itsNTimes*itsNStations*(station) 
#define CCADDRESS_TIME(freq, station, time) CCADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define CCADDRESS_POL(freq, station, time, pol) CCADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
 
 inline fcomplex* DH_CorrCube::getBuffer()
   { return itsBuffer; }
 
 inline const fcomplex* DH_CorrCube::getBuffer() const
   { return itsBuffer; }
 
 inline fcomplex* DH_CorrCube::getBufferElement(int channel, 
						  int station,
						  int sample,
						  int pol)     
   { return itsBuffer + CCADDRESS_POL(channel, station, sample, pol); }
 
/*  inline fcomplex* getBufferTimePolSeries(int channel, int station)  */
/*    { return itsBuffer + CCADDRESS_STATION(channel, station); } */
 
 inline void DH_CorrCube::setBufferElement(int channel, 
					   int sample, 
					   int station, 
					   int polarisation,
					   fcomplex* valueptr) {
   *(itsBuffer + CCADDRESS_POL(channel, station, sample, polarisation)) = *valueptr;
 }
 
 inline const unsigned int DH_CorrCube::getBufSize() const {
   return itsBufSize;
 }
 
}
#endif 
