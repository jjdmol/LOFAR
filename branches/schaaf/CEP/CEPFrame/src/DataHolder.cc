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
//#  $Log$
//#  Revision 1.19  2002/12/13 14:01:25  schaaf
//#
//#  %[BugId: 146]%
//#  Added the writedelay counter
//#
//#  Revision 1.18  2002/06/10 09:44:15  diepen
//#
//#  %[BugId: 37]%
//#  setRead and setWrite have been replaced by setReadDelay.
//#
//#  Revision 1.17  2002/05/15 14:54:53  wierenga
//#  Replace cout's with cdebug(x)'s.
//#
//#  Revision 1.16  2002/05/08 14:10:57  wierenga
//#  Added allocate/deallocate methods
//#
//#  Revision 1.15  2002/05/03 11:21:31  gvd
//#  Changed for new build environment (mostly added package name to include)
//#
//#  Revision 1.14  2002/03/14 14:18:56  wierenga
//#  system include before local includes
//#
//#  Revision 1.13  2002/03/01 08:27:56  gvd
//#  Replaced firewall by Debug and changed code accordingly
//#  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//#
//#  Revision 1.12  2001/09/24 14:04:08  gvd
//#  Added preprocess and postprocess functions
//#
//#  Revision 1.11  2001/09/21 12:19:02  gvd
//#  Added make functions to WH classes to fix memory leaks
//#
//#  Revision 1.10  2001/03/23 10:00:40  gvd
//#  Improved documentation and test programs
//#  Added clearEventCount function to Step
//#
//#  Revision 1.9  2001/03/01 13:15:47  gvd
//#  Added type argument in DataHolder constructor which is used in
//#  the connect functions to check if the DH types match
//#  Improved the simulator parser
//#  Improved documentation
//#
//#  Revision 1.8  2001/02/05 14:53:04  loose
//#  Added GPL headers
//#
//#////////////////////////////////////////////////////////////////////

#include <Common/lofar_iostream.h>
#include <stdexcept>

#include "BaseSim/Transport.h"
#include "BaseSim/DataHolder.h"
#include "BaseSim/Transport.h"
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
