//  DH_GrowSize.h: DataHolder with one dimensional byte array that 
//                  can grow its size (e.g. for performance measurements)
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
//
//////////////////////////////////////////////////////////////////////

#ifndef DH_GROWSIZE_H
#define DH_GROWSIZE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/DataHolder.h"
#include <Common/Debug.h>
#include <Common/lofar_complex.h>

/**
   This class is an data holder that increases its size
*/

class DH_GrowSize: public DataHolder
{
public:
  typedef int BufferType;

  DH_GrowSize (const string& name, unsigned int nbuffer, bool sizeFixed);

  virtual ~DH_GrowSize();

  DataHolder* clone() const;

  virtual void preprocess();
  virtual void postprocess();

  /// Set the Counter attribute in the DataPacket.
  void setCounter (int counter);
  /// Get the Counter attribute in the DataPacket.
  int getCounter() const;

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  // increaseSize: increase the size returned by getDataPacketSize
  bool increaseSize(float factor);

  virtual void dump();

  // override getDataPacketSize accessor methods
  int getDataPacketSize();
  int getCurDataPacketSize();
  int getMaxDataPacketSize();

  bool setInitialDataPacketSize(int initialSize);

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};
    int itsCounter;
    BufferType* itsBuffer;
  };

  DH_GrowSize (const DH_GrowSize&);

private:

  /// Forbid assignment.
  DH_GrowSize& operator= (const DH_GrowSize&);

  // value of nbuffer argument to constructor
  // used preprocess to allocate correct amount of memory
  int itsBufSize;

  // pointer to a dataPacket object
  DataPacket* itsDataPacket;

  // keep track of the reported sizes of the getCurDataPacketSize()
  // method. This value is increased in the increaseSize() method
  float reportedDataPacketSize;

  /// fixed size?
  bool itsSizeFixed;
};

inline void DH_GrowSize::setCounter (int counter)
  { itsDataPacket->itsCounter = counter; }

inline int DH_GrowSize::getCounter() const
  { return itsDataPacket->itsCounter; }

inline DH_GrowSize::BufferType* DH_GrowSize::getBuffer()
  { return itsDataPacket->itsBuffer; }

inline const DH_GrowSize::BufferType* DH_GrowSize::getBuffer() const
  { return itsDataPacket->itsBuffer; }

inline bool DH_GrowSize::increaseSize(float factor) { 
  if (reportedDataPacketSize * factor < this->DataHolder::getDataPacketSize()){
    reportedDataPacketSize *= factor;
    return true;
  }
  return false;
}

inline int DH_GrowSize::getDataPacketSize(void) {
  return getCurDataPacketSize(); 
}

inline int DH_GrowSize::getCurDataPacketSize(void)
{ return (int)reportedDataPacketSize; }

inline int DH_GrowSize::getMaxDataPacketSize(void)
{ return DataHolder::getDataPacketSize(); }

inline bool DH_GrowSize::setInitialDataPacketSize(int initialSize){
//  if (initialSize <= this->DataHolder::getMaxDataPacketSize()) {
    if (1) {
    reportedDataPacketSize = initialSize;
    return true;
  }
  return false;
}

#endif 


