//# DH_Example.h: Example DataHolder
//#
//# Copyright (C) 2000
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

#ifndef LIBTRANSPORT_TEST_DH_EXAMPLE_H
#define LIBTRANSPORT_TEST_DH_EXAMPLE_H


#include <lofar_config.h>
#include "libTransport/DH_PL.h"
#include <Common/lofar_complex.h>
#include <PL/DBRep.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs.

class DH_Example: public DH_PL
{
public:
  typedef complex<float> BufferType;

  explicit DH_Example (const string& name, unsigned int nbuffer = 10);

  DH_Example(const DH_Example&);

  virtual ~DH_Example();

  DataHolder* clone() const;

  /// Set the Counter attribute in the DataPacket.
  void setCounter (int counter);
  /// Get the Counter attribute in the DataPacket.
  int getCounter() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

private:
  /// Forbid assignment.
  DH_Example& operator= (const DH_Example&);


  // Data pointers filled by preprocess.
  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
};


inline void DH_Example::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_Example::getCounter() const
  { return *itsCounter; }

inline DH_Example::BufferType* DH_Example::getBuffer()
  { return itsBuffer; }

inline const DH_Example::BufferType* DH_Example::getBuffer() const
  { return itsBuffer; }

}



namespace LOFAR {
  namespace PL {

    class DBRep<DH_Example> : public DBRep<DH_PL>
    {
    public:
      void bindCols (dtl::BoundIOs& cols);
      void toDBRep (const DH_Example&);

    private:
      int itsCounter;
    };
  }
}

#endif 
