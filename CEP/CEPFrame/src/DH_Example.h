//# DH_Example.h: Example DataHolder
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

#ifndef CEPFRAME_DH_EXAMPLE_H
#define CEPFRAME_DH_EXAMPLE_H


//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{

/**
   This class is an example DataHolder.
*/

class DH_Example: public DataHolder
{
public:
  typedef fcomplex BufferType;

  explicit DH_Example (const string& name, unsigned int nbuffer = 10);

  DH_Example(const DH_Example&);

  virtual ~DH_Example();

  DataHolder* clone() const;

  /// Set the Counter attribute in the DataPacket.
  void setCounter (int counter);
  /// Get the Counter attribute in the DataPacket.
  int getCounter() const;

  /// Aloocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

 /// overload the getcursize method;
  /// reported data size may be altered with setDataPacketSize() method
  int  getCurDataSize() ;
  void setCurDataSize(const int nbytes) ;

private:
  /// Forbid assignment.
  DH_Example& operator= (const DH_Example&);

  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  int*         itsCounter;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
  int          itsCurDataPacketSize;
};


inline void DH_Example::setCounter (int counter)
  { *itsCounter = counter; }

inline int DH_Example::getCounter() const
  { return *itsCounter; }

inline DH_Example::BufferType* DH_Example::getBuffer()
  { return itsBuffer; }

inline const DH_Example::BufferType* DH_Example::getBuffer() const
  { return itsBuffer; }

inline int DH_Example::getCurDataSize() 
  { return itsCurDataPacketSize; }
   
inline void DH_Example::setCurDataSize(const int nbytes)
  {  
    //! DBGASSERTSTR(nbytes <= (int)itsBufSize); 
    itsCurDataPacketSize = nbytes;
    return ;
  }

}

#endif 
