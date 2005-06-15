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


#include <ACC/ParameterSet.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_RSP (const string& name,
                   const ACC::ParameterSet pset);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// accessor functions to the blob data
  const int getStationID() const;
  void setStationID(int);
  const int getSeqID() const;
  void setSeqID(int);
  const int getBlockID() const;
  void setBlockID(int);
  const int getFlag() const;
  void setFlag(int);
  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  /// Reset the buffer
  void resetBuffer();
  
 private:
  /// Forbid assignment.
  DH_RSP& operator= (const DH_RSP&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  BufferType*  itsBuffer;
  int* itsFlagPtr;

  int itsEPAheaderSize;
  int itsNoBeamlets;
  int itsNoPolarisations;
  unsigned int itsBufSize;

  ACC::ParameterSet itsPSet;
};

inline DH_RSP::BufferType* DH_RSP::getBuffer()
  { return itsBuffer; }
   
inline const DH_RSP::BufferType* DH_RSP::getBuffer() const
  { return itsBuffer; }

inline const int DH_RSP::getStationID() const
  { return ((int*)&itsBuffer[2])[0]; }

inline const int DH_RSP::getSeqID() const
  { return ((int*)&itsBuffer[6])[0]; }

inline const int DH_RSP::getBlockID() const
  { return ((int*)&itsBuffer[10])[0]; }

inline const int DH_RSP::getFlag() const
  { return *itsFlagPtr; }

inline void DH_RSP::setStationID(int stationid)
  { memcpy(&itsBuffer[2], &stationid, sizeof(int)); }

inline void  DH_RSP::setSeqID(int seqid)
  { memcpy(&itsBuffer[6], &seqid, sizeof(int)); }

inline void  DH_RSP::setBlockID(int blockid)
  { memcpy(&itsBuffer[10], &blockid, sizeof(int)); }

inline void  DH_RSP::setFlag(int flag)
  { *itsFlagPtr = flag; }

inline void DH_RSP:: resetBuffer()
  { memset(itsBuffer, 0, itsBufSize); }


}

#endif 
