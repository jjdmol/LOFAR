//# DH_SubBand.h: SubBand DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_SUBBAND_H
#define TFLOPCORRELATOR_DH_SUBBAND_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_SubBand: public DataHolder
{
public:
  typedef uint16 BufferPrimitive;
  typedef complex<BufferPrimitive> BufferType;

  explicit DH_SubBand (const string& name); 


  DH_SubBand(const DH_SubBand&);

  virtual ~DH_SubBand();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  LOFAR_BUILTIN_COMPLEXFP* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const LOFAR_BUILTIN_COMPLEXFP* getBuffer() const;

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop
  LOFAR_BUILTIN_COMPLEXFP* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  LOFAR_BUILTIN_COMPLEXFP* getBufferElement(int channel, int station, int sample, int polarisation);
  void setBufferElement(int channel, int station, int sample, int polarisation, BufferType* value); 

   const unsigned int getBufSize() const;

private:
  /// Forbid assignment.
  DH_SubBand& operator= (const DH_SubBand&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  void fillDataPointers();
};


#define SBADDRESS_STATION (freq, station)       CCADDRESS_FREQ((freq)) +  itsNPol*itsNTimes*itsNStations*(station) 
#define SBADDRESS_TIME    (freq, station, time) CCADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define SBADDRESS_POL     (freq, station, time, pol) CCADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
 
 inline LOFAR_BUILTIN_COMPLEXFP* DH_SubBand::getBuffer()
   { return itsBuffer; }
 
 inline const LOFAR_BUILTIN_COMPLEXFP* DH_SubBand::getBuffer() const
   { return itsBuffer; }
 
 inline LOFAR_BUILTIN_COMPLEXFP* DH_SubBand::getBufferElement(int channel, 
							       int station,
							       int sample,
							       int pol)     
   { return itsBuffer + CCADDRESS_POL(channel, station, sample, pol); }
 
 LOFAR_BUILTIN_COMPLEXFP* getBufferTimePolSeries(int channel, int station) 
   { return itsBuffer + CCADDRESS_STATION(channel, station); } 
 
 inline void DH_SubBand::setBufferElement(int channel, 
					   int sample, 
					   int station, 
					   int polarisation,
					   LOFAR_BUILTIN_COMPLEXFP* valueptr) {
   *(itsBuffer + CCADDRESS_POL(channel, station, sample, pol)) = *valueptr;
 }
 
 inline const unsigned int DH_SubBand::getBufSize() const {
   return itsBufSize;
 }
 
}

#endif 
