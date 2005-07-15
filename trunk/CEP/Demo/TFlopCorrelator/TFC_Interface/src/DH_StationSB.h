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
#include <APS/ParameterSet.h>

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
			 const short   subband,
			 const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_StationSB(const DH_StationSB&);

  virtual ~DH_StationSB();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer in the DataPacket.
  u16complex* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const u16complex* getBuffer() const;

  const unsigned int getBufferSize() const;

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

  ACC::APS::ParameterSet itsPset;
  
  void fillDataPointers();
};


inline DH_StationSB::BufferType* DH_StationSB::getBuffer()
  { return itsBuffer; }
 
inline const DH_StationSB::BufferType* DH_StationSB::getBuffer() const
  { return itsBuffer; }
 
inline const unsigned int DH_StationSB::getBufferSize() const 
  { return itsBufSize; }

inline RectMatrix<DH_StationSB::BufferType>& DH_StationSB::getDataMatrix() const 
  { return *itsMatrix; }
 
}
#endif 
