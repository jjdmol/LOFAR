//# DH_Example.h: Example DataHolder storing no individual fields
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

#ifndef LIBTRANSPORT_TEST_DH_EXAMPLE_H
#define LIBTRANSPORT_TEST_DH_EXAMPLE_H


#include <lofar_config.h>
#include <TransportPL/DH_PL.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs.

class DH_Example: public DH_PL
{
public:
  typedef fcomplex BufferType;

  explicit DH_Example (const string& name="dh_example",
		       unsigned int initialNelements = 10,
		       bool useExtra = false);

  DH_Example(const DH_Example&);

  virtual ~DH_Example();

  DataHolder* clone() const;

  /// Set the Counter attribute.
  void setCounter (int counter);
  /// Get the Counter attribute.
  int getCounter() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Reset the buffer size.
  void setBufferSize (unsigned int nelements);

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

 private:
  /// Forbid assignment.
  DH_Example& operator= (const DH_Example&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();


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

#endif 
