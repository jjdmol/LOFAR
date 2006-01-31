//# DH_RSP.h: DataHolder storing RSP raw ethernet frames for 
//#           StationCorrelator demo
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RSP_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RSP_H


#include <APS/ParameterSet.h>
#include <Transport/DataHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/RectMatrix.h>

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef i16complex BufferType;

  explicit DH_RSP (const string &name,
                   const ACC::APS::ParameterSet &pset);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder *clone() const;

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

  const ACC::APS::ParameterSet &itsPSet;

  RectMatrix<BufferType> *itsMatrix;
};

inline DH_RSP::BufferType *DH_RSP::getBuffer()
  { return itsBuffer; }

inline uint DH_RSP::getBufferSize() const
  { return itsBufSize; }
   
inline const DH_RSP::BufferType *DH_RSP::getBuffer() const
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

inline RectMatrix<DH_RSP::BufferType> &DH_RSP::getDataMatrix() const 
  { return *itsMatrix; }

}

#endif 
