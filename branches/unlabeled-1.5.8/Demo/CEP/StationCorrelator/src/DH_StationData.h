//#  DH_StationData.h: StationData DataHolder
//#
//#  Copyright (C) 2002-2004
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

#ifndef STATIONCORRELATOR_DH_STATIONDATA_H
#define STATIONCORRELATOR_DH_STATIONDATA_H

//# Includes
#include <Transport/DataHolder.h>
#include <complex>

using std::complex;

namespace LOFAR
{
  // Description of class.
class DH_StationData: public DataHolder
{
 public:
  typedef complex<int16> BufferType;

  explicit DH_StationData (const string& name,
                           const unsigned int bufsize);
  virtual ~DH_StationData();

  DH_StationData(const DH_StationData&);
  DataHolder* clone() const;

  /// Allocate the buffers
  virtual void preprocess();
  
  /// Deallocate the buffers
  virtual void postprocess();

  
  /// accessor functions to the blob data
  int getStationID() const;
  void setStationID(int);
  int getBlockID() const;
  void setBlockID(int);
  int getFlag() const;
  void setFlag(int);
  BufferType* getBuffer(); 

  unsigned int itsBufSize; 
 
 private:
  /// forbid assignment
  DH_StationData& operator= (const DH_StationData&);

  /// pointers to data in the blob
  int* itsStationIDptr;
  int* itsBlockIDptr;
  int* itsFlagptr;
  BufferType* itsBuffer;
  
  void fillDataPointers();

};

inline int DH_StationData::getStationID() const 
  { return *itsStationIDptr; }

inline void DH_StationData::setStationID(int stationid)  
  { *itsStationIDptr = stationid; }

inline int DH_StationData::getBlockID() const 
  { return *itsBlockIDptr; }

inline void DH_StationData::setBlockID(int blockid)  
  { *itsBlockIDptr = blockid; }

inline int DH_StationData::getFlag() const 
  { return *itsStationIDptr; }

inline void DH_StationData::setFlag(int flag)  
  { *itsFlagptr = flag; }

inline DH_StationData::BufferType* DH_StationData::getBuffer() 
  { return itsBuffer; }


} // namespace LOFAR

#endif
