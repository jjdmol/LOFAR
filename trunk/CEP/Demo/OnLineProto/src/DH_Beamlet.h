//# DH_Beamlet.h: Beamlet DataHolder
//#
//# Copyright (C) 2000, 2001
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

#ifndef ONLINEPROTO_DH_BEAMLET_H
#define ONLINEPROTO_DH_BEAMLET_H


#include <lofar_config.h>

#include "CEPFrame/DataHolder.h"
#include <Common/lofar_complex.h>



namespace LOFAR
{


/**
   TBW
*/
class DH_Beamlet: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Beamlet (const string& name, 
		       const int FBW);

  DH_Beamlet(const DH_Beamlet&);

  virtual ~DH_Beamlet();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int freq);
  const int         getFBW() const;
protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket(){};

    int itsStationID;       // source station ID
    int itsBeamID;          // beam direction ID
    int itsStartFrequency;  // frequency offste for this beamlet
    int itsSeqNo;           // sequence number since last timestamp

    BufferType itsFill;         // to ensure alignment
  };

private:
  /// Forbid assignment.
  DH_Beamlet& operator= (const DH_Beamlet&);


  DataPacket*  itsDataPacket;
  BufferType*  itsBuffer;    // array containing frequency spectrum.
  unsigned int itsBufSize;
  
  int          itsFBW; // number of frequency channels within this beamlet

};

inline DH_Beamlet::BufferType* DH_Beamlet::getBuffer()
  { return itsBuffer; }

inline const DH_Beamlet::BufferType* DH_Beamlet::getBuffer() const
  { return itsBuffer; }

inline DH_Beamlet::BufferType* DH_Beamlet::getBufferElement(int freq)
{ 
  return itsBuffer+freq;
}

inline const int DH_Beamlet::getFBW() const
  { return itsFBW; }

}

#endif 
