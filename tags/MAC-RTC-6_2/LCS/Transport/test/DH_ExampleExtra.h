//# DH_ExampleExtra.h: Example DataHolder storing no individual fields
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

#ifndef LIBTRANSPORT_TEST_DH_EXAMPLEEXTRA_H
#define LIBTRANSPORT_TEST_DH_EXAMPLEEXTRA_H


#include <lofar_config.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs. This DataHolder shows how to use the extra blob
// to store variable sized and different types of data in.

class DH_ExampleExtra: public DataHolder
{
public:
  typedef fcomplex BufferType;

  explicit DH_ExampleExtra (const string& name="dh_example",
		       unsigned int initialNelements = 10);

  DH_ExampleExtra(const DH_ExampleExtra&);

  virtual ~DH_ExampleExtra();

  virtual DataHolder* clone() const;

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

  /// Get a BlobOStream object to fill the variable buffer (extra blob).
  BlobOStream& fillVariableBuffer();

  /// Get a BlobIStream object to read from the variable buffer.
  BlobIStream& readVariableBuffer();
  BlobIStream& readVariableBuffer(bool& found, int& version);

  /// Clear the variable buffer.
  void clearVariableBuffer();

 private:
  /// Forbid assignment.
  DH_ExampleExtra& operator= (const DH_ExampleExtra&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();


  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
};


inline void DH_ExampleExtra::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_ExampleExtra::getCounter() const
  { return *itsCounter; }

inline DH_ExampleExtra::BufferType* DH_ExampleExtra::getBuffer()
  { return itsBuffer; }
   
inline const DH_ExampleExtra::BufferType* DH_ExampleExtra::getBuffer() const
  { return itsBuffer; }

inline BlobOStream& DH_ExampleExtra::fillVariableBuffer()
  { return createExtraBlob(); }

inline BlobIStream& DH_ExampleExtra::readVariableBuffer()
  { return getExtraBlob(); }

inline BlobIStream& DH_ExampleExtra::readVariableBuffer(bool& found, int& version)
  { return getExtraBlob(found, version); }

inline void DH_ExampleExtra::clearVariableBuffer()
  { clearExtraBlob(); }
}

#endif 
