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
//  $Log$
//  Revision 1.10  2002/07/19 13:52:44  wierenga
//
//  %[BugId: 73]%
//
//  Add fixedsize parameter to allow fixed message size transfers tests.
//
//  Revision 1.9  2002/05/08 14:28:37  wierenga
//  DataHolder allocation moved from constructor to preprocess to be able to
//  use TransportHolder::allocate.
//  Bug fixes in P2Perf.cc for -mpi arguments.
//
//  Revision 1.8  2002/05/08 08:20:04  schaaf
//  Modified includes for new build env
//
//  Revision 1.7  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.6  2002/04/09 10:59:54  schaaf
//  minor
//
//  Revision 1.5  2002/03/27 09:51:13  schaaf
//  Use get{Cur/Max}DataPacketSize
//
//  Revision 1.3  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.2  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.1  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////

#ifndef DH_GROWSIZE_H
#define DH_GROWSIZE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/DataHolder.h"
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

private:
  /// Forbid copy constructor.
  DH_GrowSize (const DH_GrowSize&);
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


