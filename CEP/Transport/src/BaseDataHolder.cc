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

namespace LOFAR
{

BaseDataHolder::BaseDataHolder(const &string name, const &string type)
  : itsTransporter      ()
    itsName             (name),
    itsType             (type),
    itsReadDelay        (0),
    itsWriteDelay       (0),
    ItsReadDelayCount   (0),
    itsWriteDelayCount  (0),
    itaIfsPtr           (0),
    itsOfsPtr           (0)
{
  setDefaultDataPacket();
}

DataHolder::DataHolder(const DataHolder& that)
  : itsDataPacketSize     (that.itsDataPacketSize),
    itsName               (that.itsName),
    itsType               (that.itsType),
    itsReadDelay          (that.itsReadDelay),
    itsWriteDelay         (that.itsWriteDelay),
    itsReadDelayCount     (that.itsReadDelayCount),
    itsWriteDelayCount    (that.itsWriteDelayCount),
    itsIfsPtr             (0),
    itsOfsPtr             (0)
{}
    
BaseDataHolder::~BaseDataHolder() {
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
  }
  if (itsOfsPtr) {
    itsOfsPtr->close();
    delete itsOfsPtr;
  }
}

void* BaseDataHolder::allocate(size_t size)
{
  void* mem = 0;
  if (!getTransport().getTransportHolder()) {
    mem = malloc(size);
  } else {
    cdebug(3) << "allocate "
	      << getTransport().getTransportHolder()->getType() << endl;
    mem = getTransport().getTransportHolder()->allocate(size);
  }
  return mem;
}

void BaseDataHolder::deallocate(void*& ptr)
{
  if (ptr) {
    if (!getTransport().getTransportHolder()) {
      free(ptr);
    } else {
      cdebug(3) << "deallocate " << getTransport().getTransportHolder()->getType() << endl;
      getTransport().getTransportHolder()->deallocate(ptr);
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
    itsTransportPtr->read();
  }
  // Write the possible outstanding buffers.
  for (int i=0; i<itsWriteDelay; i++) {
    itsTransportPtr->write();
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
    result = itsTransportPtr->read();
    if (itsIfsPtr) {
      doFsRead (*itsIfsPtr);
    }
  } else {
    itsReadDelayCount--;
  }
  return result;
}

void BaseDataHolder::write()
{ 
  if (itsWriteDelayCount <= 0) {
    itsTransportPtr->write();
    if (itsOfsPtr) {
      doFsWrite (*itsOfsPtr);
    }
  } else {
    itsWriteDelayCount--;
  }
}

void BaseDataHolder::setZeroes()
{
}

void BaseDataHolder::setOnes()
{
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


void BaseDataHolder::setInFile (const string& inFile)
{
  TRACER2("BaseDataHolder::setInFile(" << inFile << ")");
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
    itsIfsPtr = 0;
  }
  if (inFile.size() > 0) {
    itsIfsPtr = new ifstream(inFile.c_str());
    if (!(*itsIfsPtr)) {
      throw std::runtime_error("BaseDataHolder::setInFile() : File \"" + 
			       inFile + "\" not found");
    }
  }
}


bool BaseDataHolder::setOutFile (const string& outFile)
{
  TRACER2("BaseDataHolder::setOutFile(" << outFile << ")");
  if (itsOfsPtr) {
    itsOfsPtr->close();
    delete itsOfsPtr;
    itsOfsPtr = 0;
  }
  if (outFile.size() > 0) {
    itsOfsPtr = new ofstream(outFile.c_str());
    return (*itsOfsPtr);
  }
  return false;
}


void BaseDataHolder::unsetInFile()
{
  TRACER2("BaseDataHolder::unsetInFile()");
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
    itsIfsPtr = 0;
  }
}


void BaseDataHolder::unsetOutFile()
{
  TRACER2("BaseDataHolder::unsetOutFile()");
  if (itsOfsPtr) {
    itsOfsPtr->close();
    delete itsOfsPtr;
    itsOfsPtr = 0;
  }
}


bool BaseDataHolder::doFsRead (ifstream&)
{
  TRACER2("BaseDataHolder::doFsRead()");
  return true;
}


bool BaseDataHolder::doFsWrite (ofstream&) const
{
  TRACER2("BaseDataHolder::doFsWrite()");
  return true;
}


bool BaseDataHolder::isValid() const
{
  return itsTransportPtr->isValid();
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

bool BaseDataHolder::operator== (const BaseDataHolder& aPH)  const
{
  return itsSerial == aPH.itsSerial;
}

bool BaseDataHolder::operator!= (const BaseDataHolder& aPH) const
{
  return itsSerial != aPH.itsSerial;
}

bool BaseDataHolder::operator< (const BaseDataHolder& aPH) const
{
  return itsSerial < aPH.itsSerial;
}
} // namespace LOFAR
