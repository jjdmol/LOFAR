//# DH_StationSB.h: DataHolder containing ~1s of data for one station, one subband
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_TFC_INTERFACE_DH_STATIONSB_H
#define TFLOPCORRELATOR_TFC_INTERFACE_DH_STATIONSB_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{

/**
   DataHolder containing ~ 1 s of data for one station, one subband
*/
class DH_StationSB: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_StationSB (const string& name,
		       const short   subband); 


  DH_StationSB(const DH_StationSB&);

  virtual ~DH_StationSB();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

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
  DH_StationSB& operator= (const DH_StationSB&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubband;
  short itsNFChannels;
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  RectMatrix<BufferType>* itsMatrix;

  void fillDataPointers();
};


inline DH_StationSB::BufferType* DH_StationSB::getBuffer()
  { return itsBuffer; }
 
inline const DH_StationSB::BufferType* DH_StationSB::getBuffer() const
  { return itsBuffer; }
 
inline const unsigned int DH_StationSB::getBufSize() const 
  { return itsBufSize; }

inline RectMatrix<DH_StationSB::BufferType>& DH_StationSB::getDataMatrix() const 
  { return *itsMatrix; }
 
}
#endif 
