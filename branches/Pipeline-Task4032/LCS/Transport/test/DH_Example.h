//# DH_Example.h: Example DataHolder storing no individual fields
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LIBTRANSPORT_TEST_DH_EXAMPLE_H
#define LIBTRANSPORT_TEST_DH_EXAMPLE_H


//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs.

class DH_Example: public DataHolder
{
public:
  typedef fcomplex BufferType;

  explicit DH_Example (const string& name="dh_example",
		       unsigned int initialNelements = 10);

  DH_Example(const DH_Example&);

  virtual ~DH_Example();

  virtual DH_Example* clone() const;

  /// Set the Counter attribute.
  void setCounter (int counter);
  /// Get the Counter attribute.
  int getCounter() const;

  /// Allocate the buffers.
  /// Note: call this after this DataHolder has been connected to make 
  /// sure the buffers are allocated in the right place (in normal or 
  /// shared memory in case of connection with TH_ShMem)
  virtual void init();

  /// Reset the buffer size.
  void setBufferSize (unsigned int nelements);

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;
  /// Get Buffer element.
  BufferType& getBufferElement(unsigned int element);

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
