//  DH_IntArray.h: Example DataHolder
//
//  Copyright (C) 2000, 2001
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
//  Revision 1.3  2002/05/08 08:20:04  schaaf
//  Modified includes for new build env
//
//  Revision 1.2  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_DH_INTARRAY_H
#define BASESIM_DH_INTARRAY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/DataHolder.h"
#include <Common/lofar_complex.h>

/**
   This class is an integer array DataHolder.
*/

class DH_IntArray: public DataHolder
{
public:
  typedef int BufferType;

  DH_IntArray (const string& name, unsigned int nbuffer = 10);

  virtual ~DH_IntArray();

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
  };

private:
  /// Forbid copy constructor.
  DH_IntArray (const DH_IntArray&);
  /// Forbid assignment.
  DH_IntArray& operator= (const DH_IntArray&);


  DataPacket* itsDataPacket;
  BufferType* itsBuffer;
};


inline void DH_IntArray::setCounter (int counter)
  { itsDataPacket->itsCounter = counter; }

inline int DH_IntArray::getCounter() const
  { return itsDataPacket->itsCounter; }

inline DH_IntArray::BufferType* DH_IntArray::getBuffer()
  { return itsBuffer; }

inline const DH_IntArray::BufferType* DH_IntArray::getBuffer() const
  { return itsBuffer; }


#endif 
