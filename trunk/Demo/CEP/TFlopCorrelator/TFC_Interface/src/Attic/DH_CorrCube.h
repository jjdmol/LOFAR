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
  typedef fcomplex BufferType;

  explicit DH_CorrCube (const string& name, short subband); 


  DH_CorrCube(const DH_CorrCube&);

  virtual ~DH_CorrCube();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  void setBuffer(BufferType*);

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop
  BufferType* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  BufferType* getBufferElement(int channel, int station, int sample, int polarisation);
  void setBufferElement(int channel, int station, int sample, int polarisation, BufferType* value); 

  const unsigned int getBufSize() const;

  void setTestPattern();
  void print();

private:
  size_t offset(int channel, int station, int sample, int polarisation);

  /// Forbid assignment.
  DH_CorrCube& operator= (const DH_CorrCube&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNFChannels;     // #frequency channels per buffer
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  void fillDataPointers();
};


#if 0
#define CCADDRESS_FREQ(freq) itsNPol*itsNTimes*itsNStations*itsNFChannels*(freq)
#define CCADDRESS_STATION(freq, station) CCADDRESS_FREQ((freq)) +  itsNPol*itsNTimes*itsNStations*(station) 
#define CCADDRESS_TIME(freq, station, time) CCADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define CCADDRESS_POL(freq, station, time, pol) CCADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
#endif
 
 inline DH_CorrCube::BufferType* DH_CorrCube::getBuffer()
   { return itsBuffer; }
 
 inline const DH_CorrCube::BufferType* DH_CorrCube::getBuffer() const
   { return itsBuffer; }
 
 inline void DH_CorrCube::setBuffer(DH_CorrCube::BufferType* buffer)
   { itsBuffer = buffer; }
   

  inline size_t DH_CorrCube::offset(int channel, int station, int sample, int polarisation)
  {
    return (((size_t) channel * itsNStations + station) * itsNTimes + sample) * itsNPol + polarisation;
  }

 inline DH_CorrCube::BufferType* DH_CorrCube::getBufferElement(int channel, 
						  int station,
						  int sample,
						  int pol)     
   //{ return itsBuffer + CCADDRESS_POL(channel, station, sample, pol); }
   { return itsBuffer + offset(channel, station, sample, pol); }
 
/*  inline BufferType* getBufferTimePolSeries(int channel, int station)  */
/*    { return itsBuffer + CCADDRESS_STATION(channel, station); } */
 
 inline void DH_CorrCube::setBufferElement(int channel, 
					   int sample, 
					   int station, 
					   int polarisation,
					   BufferType* valueptr) {
   //*(itsBuffer + CCADDRESS_POL(channel, station, sample, polarisation)) = *valueptr;
   { itsBuffer[offset(channel, station, sample, polarisation)] = *valueptr; }
 }
 
 inline const unsigned int DH_CorrCube::getBufSize() const {
   return itsBufSize;
 }

 inline void DH_CorrCube::setTestPattern() { 
   for (unsigned int i = 0; i < itsBufSize; i++) {
     itsBuffer[i] = makefcomplex(1.0f, 1.0f);
   }
 }

}
#endif 
