//  DH_Tester.h:
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
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_DH_TESTER_H
#define CEPFRAME_DH_TESTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/lofar_complex.h>
#include "CEPFrame/DataHolder.h"
#include "CEPFrame/BaseSim.h"

namespace LOFAR
{

/**
   This class is a simple DataHolder test class for the program Tester.
*/

class DH_Tester: public DataHolder
{
public:
  typedef complex<float> DataBufferType;

  explicit DH_Tester (const string& name);
  DH_Tester (const DH_Tester&);
  virtual ~DH_Tester();

  DataHolder* clone() const;

  void setCounter (int counter);
  int getCounter() const;

  DataBufferType* getBuffer();
  const DataBufferType* getBuffer() const;

protected:
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};
    DataBufferType itsBuffer[10];
    int itsCounter;
  };

private:
  /// Forbid assignment.
  DH_Tester& operator= (const DH_Tester&);


  DataPacket itsDataPacket;
};


inline void DH_Tester::setCounter (int counter)
  { itsDataPacket.itsCounter = counter; }

inline int DH_Tester::getCounter() const
  { return itsDataPacket.itsCounter; }

inline DH_Tester::DataBufferType* DH_Tester::getBuffer()
  { return itsDataPacket.itsBuffer; }

inline const DH_Tester::DataBufferType* DH_Tester::getBuffer() const
  { return itsDataPacket.itsBuffer; }

}

#endif 
