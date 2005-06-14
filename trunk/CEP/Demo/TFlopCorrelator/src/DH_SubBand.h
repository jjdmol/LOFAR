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

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <RectMatrix.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_SubBand: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_SubBand (const string& name,
		       const short   subband); 


  DH_SubBand(const DH_SubBand&);

  virtual ~DH_SubBand();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  u16complex* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const u16complex* getBuffer() const;

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop

  //todo: define usefull accessors 
  //BufferType* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  BufferType* getBufferElement(int channel, int station, int sample, int polarisation);
  //void setBufferElement(int channel, int station, int sample, int polarisation, BufferType* value); 

   const unsigned int getBufSize() const;

   RectMatrix<BufferType>& getDataMatrix() const;

private:
  /// Forbid assignment.
  DH_SubBand& operator= (const DH_SubBand&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNFChannels;
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  RectMatrix<BufferType>* itsMatrix;

  void fillDataPointers();
};

//exact addressing to be defined 
#define SBADDRESS_STATION(freq, station)        itsNPol*itsNTimes*itsNStations*(station) 
#define SBADDRESS_TIME(freq, station, time)     SBADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define SBADDRESS_POL(freq, station, time, pol) SBADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
 
 inline DH_SubBand::BufferType* DH_SubBand::getBuffer()
   { return itsBuffer; }
 
 inline const DH_SubBand::BufferType* DH_SubBand::getBuffer() const
   { return itsBuffer; }
 

 inline DH_SubBand::BufferType* DH_SubBand::getBufferElement(int channel,
							     int station,
							     int sample,
							     int pol)
   { return itsBuffer + SBADDRESS_POL(channel, station, sample, pol); }
 
/*  DH_SubBand::BufferType* getBufferTimePolSeries(int channel, int station) */
/*    { return itsBuffer + SBADDRESS_STATION(channel, station); } */
 
/*  inline void DH_SubBand::setBufferElement(int channel, */
/* 					  int sample, */
/* 					  int station, */
/* 					  int polarisation, */
/* 					  DH_SubBand::BufferType* valueptr) { */
/*    *(itsBuffer + SBADDRESS_POL(channel, station, sample, pol)) = *valueptr; */
/*  } */
 
 inline const unsigned int DH_SubBand::getBufSize() const {
   return itsBufSize;
 }

 inline RectMatrix<DH_SubBand::BufferType>& DH_SubBand::getDataMatrix() const {
   return *itsMatrix; 
 };
 
}
#endif 
