//#  BaseDataHolder.cc: A parent class for DataHolder and Paramholder
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <BaseDataHolder.h>
#include <Common/Debug.h>

namespace LOFAR
{

BaseDataHolder::BaseDataHolder(const string& name, const string& type)
  : itsTransporter      (),
    itsName             (name),
    itsType             (type),
    itsReadDelay        (0),
    itsWriteDelay       (0),
    itsReadDelayCount   (0),
    itsWriteDelayCount  (0)
{
  setDefaultDataPacket();
}

BaseDataHolder::BaseDataHolder(const BaseDataHolder& that)
  : itsDataPacketSize     (that.itsDataPacketSize),
    itsName               (that.itsName),
    itsType               (that.itsType),
    itsReadDelay          (that.itsReadDelay),
    itsWriteDelay         (that.itsWriteDelay),
    itsReadDelayCount     (that.itsReadDelayCount),
    itsWriteDelayCount    (that.itsWriteDelayCount)
{}
    
BaseDataHolder::~BaseDataHolder() 
{
}

void* BaseDataHolder::allocate(size_t size)
{
  void* mem = 0;
  if (!getTransporter().getTransportHolder()) {
    mem = malloc(size);
  } else {
    cdebug(3) << "allocate "
	      << getTransporter().getTransportHolder()->getType() << endl;
    mem = getTransporter().getTransportHolder()->allocate(size);
  }
  return mem;
}

void BaseDataHolder::deallocate(void*& ptr)
{
  if (ptr) {
    if (!getTransporter().getTransportHolder()) {
      free(ptr);
    } else {
      cdebug(3) << "deallocate " << getTransporter().getTransportHolder()->getType() << endl;
      getTransporter().getTransportHolder()->deallocate(ptr);
    }
  }
  ptr = 0;
}

void BaseDataHolder::basePreprocess()
{
  itsReadDelayCount = itsReadDelay;
  itsWriteDelayCount = itsWriteDelay;
  preprocess();
}

void BaseDataHolder::preprocess()
{}

void BaseDataHolder::basePostprocess()
{
  // Read the possible outstanding buffers.
  for (int i=0; i<itsReadDelay; i++) {
    itsTransporter->read();
  }
  // Write the possible outstanding buffers.
  for (int i=0; i<itsWriteDelay; i++) {
    itsTransporter->write();
  }
  postprocess();
}

void BaseDataHolder::postprocess()
{}


void BaseDataHolder::dump() const
{
  TRACER2("BaseDataHolder dump: " << itsType << ' ' << itsName);
}

bool BaseDataHolder::read()
{
  bool result = false;
  if (itsReadDelayCount <= 0) {
    result = itsTransporter->read();
  } else {
    itsReadDelayCount--;
  }
  return result;
}

void BaseDataHolder::write()
{ 
  if (itsWriteDelayCount <= 0) {
    itsTransporter->write();
  } else {
    itsWriteDelayCount--;
  }
}

int BaseDataHolder::DataPacket::compareTimeStamp (const DataPacket& that) const
{
  if (itsTimeStamp == that.itsTimeStamp) {
    return 0;
  } else if (itsTimeStamp < that.itsTimeStamp) {
    return -1;
  }
  return 1;
}

bool BaseDataHolder::isValid() const
{
  return itsTransporter->isValid();
}

int BaseDataHolder::getNode() const
{
  return 1;
} 

void BaseDataHolder::setReadDelay (int delay)
{
  itsReadDelay = delay;
  itsReadDelayCount = delay;
}

void BaseDataHolder::setWriteDelay (int delay)
{
  itsWriteDelay = delay;
  itsWriteDelayCount = delay;
}

} // namespace LOFAR
