//# DH_VarBuf.h: Example DataHolder storing no individual fields
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

#ifndef LIBTRANSPORT_TEST_DH_VARBUF_H
#define LIBTRANSPORT_TEST_DH_VARBUF_H


//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs. It shows how to create a user defined, variable size
// datafield in the blob.

class DH_VarBuf: public DataHolder
{
public:
  typedef fcomplex BufferType;

  explicit DH_VarBuf (const string& name="dh_example");

  DH_VarBuf(const DH_VarBuf&);

  virtual ~DH_VarBuf();

  virtual DataHolder* clone() const;

  /// Set the Counter attribute.
  void setCounter (int counter);
  /// Get the Counter attribute.
  int getCounter() const;

  /// Allocate the buffers.
  virtual void init();

  /// Reset the buffer size.
  void setBufferSize (unsigned int nelements);
  unsigned int getBufferSize();

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;
  /// Get Buffer element.
  BufferType& getBufferElement(unsigned int element);

 private:
  /// Forbid assignment.
  DH_VarBuf& operator= (const DH_VarBuf&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();


  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
};


inline void DH_VarBuf::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_VarBuf::getCounter() const
  { return *itsCounter; }

inline DH_VarBuf::BufferType* DH_VarBuf::getBuffer()
  { return itsBuffer; }
   
inline const DH_VarBuf::BufferType* DH_VarBuf::getBuffer() const
  { return itsBuffer; }


}

#endif 
