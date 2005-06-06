//# DH_Buffer.h: Example DataHolder storing no individual fields
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

#ifndef ASYNCTEST_DH_EXAMPLE_H
#define ASYNCTEST_DH_EXAMPLE_H


#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

// This class is contains a buffer of a fixed size.

class DH_Buffer: public DataHolder
{
public:

  explicit DH_Buffer (const string& name="dh_example",
		       unsigned int initialNelements = 256);

  DH_Buffer(const DH_Buffer&);

  virtual ~DH_Buffer();

  virtual DH_Buffer* clone() const;

  /// Set the Counter attribute.
  void setCounter (int counter);
  /// Get the Counter attribute.
  int getCounter() const;

  /// Allocate the buffers.
  /// Note: call this after this DataHolder has been connected to make 
  /// sure the buffers are allocated in the right place (in normal or 
  /// shared memory in case of connection with TH_ShMem)
  virtual void init();

  /// Get write access to the Buffer.
  char* getBuffer();
  /// Get read access to the Buffer.
  const char* getBuffer() const;
  /// Get Buffer element.
  char getBufferElement(unsigned int element);

  /// Print first character from buffer
  void dump();

 private:
  /// Forbid assignment.
  DH_Buffer& operator= (const DH_Buffer&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*         itsCounter;
  char*  itsBuffer;
  unsigned int itsBufSize;
};

inline void DH_Buffer::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_Buffer::getCounter() const
  { return *itsCounter; }

inline char* DH_Buffer::getBuffer()
  { return itsBuffer; }
   
inline const char* DH_Buffer::getBuffer() const
  { return itsBuffer; }

inline void DH_Buffer::dump()
  { cout << *itsBuffer << endl; }

}

#endif 
