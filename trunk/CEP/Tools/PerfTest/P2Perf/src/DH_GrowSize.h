//  DH_GrowSize.h: Example DataHolder
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

#include "DataHolder.h"
#include <complex>

/**
   This class is an data holder that increases its size
*/

class DH_GrowSize: public DataHolder
{
public:
  typedef int BufferType;

  DH_GrowSize (const string& name, unsigned int nbuffer);

  virtual ~DH_GrowSize();

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

  // override getDataPacketSize
  int getDataPacketSize();
  bool setInitialDataPacketSize(int initialSize);

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};

    int itsCounter;
  };

private:
  /// Forbid copy constructor.
  DH_GrowSize (const DH_GrowSize&);
  /// Forbid assignment.
  DH_GrowSize& operator= (const DH_GrowSize&);

  DataPacket* itsDataPacket;
  BufferType* itsBuffer;

  int reportedDataPacketSize;
  float floatDataPacketSize;
};

inline void DH_GrowSize::setCounter (int counter)
  { itsDataPacket->itsCounter = counter; }

inline int DH_GrowSize::getCounter() const
  { return itsDataPacket->itsCounter; }

inline DH_GrowSize::BufferType* DH_GrowSize::getBuffer()
  { return itsBuffer; }

inline const DH_GrowSize::BufferType* DH_GrowSize::getBuffer() const
  { return itsBuffer; }

inline bool DH_GrowSize::increaseSize(float factor)
{ 
  bool success = false;
  
  if (reportedDataPacketSize * factor < this->DataHolder::getDataPacketSize())
  {
    floatDataPacketSize *= factor;
    reportedDataPacketSize = (int)floatDataPacketSize;
    success = true;
  }

  return success;
}

inline int DH_GrowSize::getDataPacketSize(void)
{ return reportedDataPacketSize; }

inline bool DH_GrowSize::setInitialDataPacketSize(int initialSize)
{
  bool success = false;

  if (initialSize < this->DataHolder::getDataPacketSize())
  {
    floatDataPacketSize = initialSize;
    reportedDataPacketSize = initialSize;
    success = true;
  }

  return success;
}

#endif 
