//# DH_RSP.h: DataHolder storing RSP raw ethernet frames
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef STATIONCORRELATOR_DH_RSP_H
#define STATIONCORRELATOR_DH_RSP_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_RSP (const string& name,
		   const unsigned int bufsize);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Reset the buffer size.
  void setBufferSize (const unsigned int bufsize);

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  /// Get StationID from first EPA-packet in RSP-frame
  const int getStationID() const;

  /// Get SequenceID from first EPA-packet in RSP-frame
  const int getSeqID() const;

  /// Get BlockID from first EPA-packet in RSP-frame
  const int getBlockID() const;
  
 private:
  /// Forbid assignment.
  DH_RSP& operator= (const DH_RSP&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  BufferType*  itsBuffer;
  unsigned int itsBufSize;
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

}

#endif 
