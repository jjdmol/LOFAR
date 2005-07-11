//# DH_RSP.h: DataHolder storing RSP raw ethernet frames for 
//#           StationCorrelator demo
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_DH_RSP_H
#define TFLOPCORRELATOR_DH_RSP_H


#include <APS/ParameterSet.h>
#include <Transport/DataHolder.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_RSP (const string& name,
                   const ACC::APS::ParameterSet pset);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  const int getFlag() const;
  void setFlag(int);
  const timestamp_t getSyncedStamp() const;
  void setSyncedStamp(timestamp_t);

  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  /// Reset the buffer
  void resetBuffer();
  
  RectMatrix<BufferType>& getDataMatrix() const;

 private:
  /// Forbid assignment.
  DH_RSP& operator= (const DH_RSP&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  BufferType*  itsBuffer;
  int* itsFlagPtr;
  timestamp_t* itsSyncedStampPtr;

  int itsNoBeamlets;
  int itsNFChannels;
  int itsNTimes;
  int itsNoPolarisations;
  unsigned int itsBufSize;

  ACC::APS::ParameterSet itsPSet;

  RectMatrix<BufferType>* itsMatrix;

};

inline DH_RSP::BufferType* DH_RSP::getBuffer()
  { return itsBuffer; }
   
inline const DH_RSP::BufferType* DH_RSP::getBuffer() const
  { return itsBuffer; }

inline const int DH_RSP::getFlag() const
  { return *itsFlagPtr; }

inline void  DH_RSP::setFlag(int flag)
  { *itsFlagPtr = flag; }

inline const timestamp_t DH_RSP::getSyncedStamp() const
  { return *itsSyncedStampPtr; }

inline void  DH_RSP::setSyncedStamp(timestamp_t stamp)
  { *itsSyncedStampPtr = stamp; }

inline void DH_RSP::resetBuffer()
  { memset(itsBuffer, 0, itsBufSize); }

inline RectMatrix<DH_RSP::BufferType>& DH_RSP::getDataMatrix() const 
  { return *itsMatrix; }

}

#endif 
