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
#include <Common/lofar_complex.h>

namespace LOFAR
{
  // Description of class.
class DH_StationData: public DataHolder
{
 public:
  typedef uint16 BufferPrimitive;
  typedef complex<BufferPrimitive> BufferType;

  explicit DH_StationData (const string& name);
  virtual ~DH_StationData();

  DH_StationData(const DH_StationData&);
  DataHolder* clone() const;

  /// Allocate the buffers
  virtual void preprocess();
  
  /// Deallocate the buffers
  virtual void postprocess();

  /// get write access to the Buffer in the DataPacket
  BufferType* getBuffer();
  /// get read-only access to the Buffer in the DataPacket
  const BufferType* getBuffer() const;

  const unsigned int getBufSize() const;
 
 private:
  /// forbid assignment
  DH_StationData& operator= (const DH_StationData&);

  BufferType* itsBuffer;
  unsigned int itsBufSize;
  
  void fillDataPointers();
};

inline DH_StationData::BufferType* DH_StationData::getBuffer() 
  { return itsBuffer; }

inline const DH_StationData::BufferType* DH_StationData::getBuffer() const 
  { return itsBuffer; }

inline const unsigned int DH_StationData::getBufSize() const
  { return itsBufSize; }

} // namespace LOFAR

#endif
