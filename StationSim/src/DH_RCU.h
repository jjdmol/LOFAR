//  DH_RCU.h: 
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//
//////////////////////////////////////////////////////////////////////

#ifndef STATIONSIM_DH_RCU_H
#define STATIONSIM_DH_RCU_H

#include <Common/lofar_complex.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/DataHolder.h"

/**
   This class is the DataHolder holding the RCU data.
*/

class DH_RCU: public DataHolder
{
public:
  typedef double BufferType;

  DH_RCU (const string& name);

  virtual ~DH_RCU();

  /// Aloocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Set the Counter attribute in the DataPacket.
  void setCounter (int counter);
  /// Get the Counter attribute in the DataPacket.
  int getCounter() const;

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};

    int itsCounter;
    BufferType itsFill;         // to ensure alignment
  };

private:
  /// Forbid copy constructor.
  DH_RCU (const DH_RCU&);
  /// Forbid assignment.
  DH_RCU& operator= (const DH_RCU&);


  void*        itsDataPacket;
  BufferType*  itsBuffer;
};


inline void DH_RCU::setCounter (int counter)
  { static_cast<DataPacket*>(itsDataPacket)->itsCounter = counter; }

inline int DH_RCU::getCounter() const
  { return static_cast<DataPacket*>(itsDataPacket)->itsCounter; }

inline DH_RCU::BufferType* DH_RCU::getBuffer()
  { return itsBuffer; }

inline const DH_RCU::BufferType* DH_RCU::getBuffer() const
  { return itsBuffer; }


#endif 
