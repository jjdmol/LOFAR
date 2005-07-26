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

  /// Accessor functions
  const int getStationID() const;
  void setStationID(int);
  const int getInvalidCount() const;
  void setInvalidCount(int);
  const timestamp_t getTimeStamp() const;
  void setTimeStamp(timestamp_t);
  const int getDelay() const;
  void setDelay(int);

  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  uint getBufferSize() const;

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
  int* itsStationID;
  int* itsInvalidCount;
  int* itsDelay;
  timestamp_t* itsTimeStamp;

  int itsNTimes;
  int itsNoPolarisations;
  unsigned int itsBufSize;

  ACC::APS::ParameterSet itsPSet;

  RectMatrix<BufferType>* itsMatrix;

};

inline DH_RSP::BufferType* DH_RSP::getBuffer()
  { return itsBuffer; }

inline uint DH_RSP::getBufferSize() const
  { return itsBufSize; }
   
inline const DH_RSP::BufferType* DH_RSP::getBuffer() const
  { return itsBuffer; }

inline const int DH_RSP::getStationID() const
  { return *itsStationID; }

inline void DH_RSP::setStationID(int id)
  { *itsStationID = id; }

inline const int DH_RSP::getInvalidCount() const
  { return *itsInvalidCount; }

inline void DH_RSP::setInvalidCount(int count)
  { *itsInvalidCount = count; }

inline const timestamp_t DH_RSP::getTimeStamp() const
  { return *itsTimeStamp; }

inline void DH_RSP::setTimeStamp(timestamp_t timestamp)
  { *itsTimeStamp = timestamp; }

inline const int DH_RSP::getDelay() const
  { return *itsDelay; }

inline void DH_RSP::setDelay(int delay)
  { *itsDelay = delay; }

inline void DH_RSP::resetBuffer()
  { memset(itsBuffer, 0, itsBufSize); }

inline RectMatrix<DH_RSP::BufferType>& DH_RSP::getDataMatrix() const 
  { return *itsMatrix; }

}

#endif 
