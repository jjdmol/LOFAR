//#  DataHolder.cc:
//#
//#  Copyright (C) 2000, 2001
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
//#
//#
//#////////////////////////////////////////////////////////////////////

#include <Common/lofar_iostream.h>
#include <stdexcept>

#include "CEPFrame/Transport.h"
#include "CEPFrame/DataHolder.h"
#include "CEPFrame/Transport.h"
#include "Common/Debug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataHolder::DataHolder(const string& name, const string& type)
: itsTransportPtr  (0),
  itsName          (name),
  itsType          (type),
  itsReadDelay     (0),
  itsWriteDelay     (0),
  itsReadDelayCount(0),
  itsWriteDelayCount(0),
  itsIfsPtr        (0),
  itsOfsPtr        (0)
{
  setDefaultDataPacket();
  itsTransportPtr = new Transport (this);
}

DataHolder::~DataHolder()
{
  delete itsTransportPtr;
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
  }
  if (itsOfsPtr) {
    itsOfsPtr->close();
    delete itsOfsPtr;
  }
}

void* DataHolder::allocate(size_t size)
{
  void* mem = 0;
  if (! getTransport().getTransportHolder()) {
    mem = malloc(size);
  } else {
    cdebug(3) << "allocate "
	      << getTransport().getTransportHolder()->getType() << endl;
    mem = getTransport().getTransportHolder()->allocate(size);
  }
  return mem;
}

void DataHolder::deallocate(void*& ptr)
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

void DataHolder::basePreprocess()
{
  itsReadDelayCount = itsReadDelay;
  itsWriteDelayCount = itsWriteDelay;
  preprocess();
}

void DataHolder::preprocess()
{}

void DataHolder::basePostprocess()
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

void DataHolder::postprocess()
{}


void DataHolder::dump() const
{
  cout << itsType << ' ' << itsName << endl;
}

void DataHolder::read()
{
  if (itsReadDelayCount <= 0) {
    itsTransportPtr->read();
    if (itsIfsPtr) {
      doFsRead (*itsIfsPtr);
    }
  } else {
    itsReadDelayCount--;
  }
}

void DataHolder::write()
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

void DataHolder::setZeroes()
{
}

void DataHolder::setOnes()
{
}


int DataHolder::DataPacket::compareTimeStamp (const DataPacket& that) const
{
  if (itsTimeStamp == that.itsTimeStamp) {
    return 0;
  } else if (itsTimeStamp < that.itsTimeStamp) {
    return -1;
  }
  return 1;
}


void DataHolder::setInFile (const string& inFile)
{
  TRACER2("DataHolder::setInFile(" << inFile << ")");
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
    itsIfsPtr = 0;
  }
  if (inFile.size() > 0) {
    itsIfsPtr = new ifstream(inFile.c_str());
    if (!(*itsIfsPtr)) {
      throw std::runtime_error("DataHolder::setInFile() : File \"" + 
			       inFile + "\" not found");
    }
  }
}


bool DataHolder::setOutFile (const string& outFile)
{
  TRACER2("DataHolder::setOutFile(" << outFile << ")");
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


void DataHolder::unsetInFile()
{
  TRACER2("DataHolder::unsetInFile()");
  if (itsIfsPtr) {
    itsIfsPtr->close();
    delete itsIfsPtr;
    itsIfsPtr = 0;
  }
}


void DataHolder::unsetOutFile()
{
  TRACER2("DataHolder::unsetOutFile()");
  if (itsOfsPtr) {
    itsOfsPtr->close();
    delete itsOfsPtr;
    itsOfsPtr = 0;
  }
}


bool DataHolder::doFsRead (ifstream&)
{
  TRACER2("DataHolder::doFsRead()");
  return true;
}


bool DataHolder::doFsWrite (ofstream&) const
{
  TRACER2("DataHolder::doFsWrite()");
  return true;
}

bool DataHolder::doHandle() const
{
  return itsTransportPtr->doHandle();
}

bool DataHolder::isValid() const
{
  return itsTransportPtr->isValid();
}

void DataHolder::setReadDelay (int delay)
{
  itsReadDelay = delay;
  itsReadDelayCount = delay;
}

void DataHolder::setWriteDelay (int delay)
{
  itsWriteDelay = delay;
  itsWriteDelayCount = delay;
}
