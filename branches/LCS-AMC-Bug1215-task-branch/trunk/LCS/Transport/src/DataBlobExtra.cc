//# DataBlobExtra.cc:
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transport/DataBlobExtra.h>
#include <Transport/DataHolder.h>
#include <Blob/BlobField.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>
#include <Common/LofarLogger.h>
#include <vector>

namespace LOFAR
{

DataBlobExtra::DataBlobExtra(const string& name, int version, DataHolder* DH)
 : itsOut        (0),
   itsIn         (0),
   itsBufOut     (0),
   itsBufIn      (0, 0),
   itsName       (name),
   itsVersion    (version),
   itsCreateDone (false),
   itsLastDone   (0),
   itsDH         (DH)
{
  LOG_TRACE_FLOW("DataBlobExtra constructor");
}

DataBlobExtra::~DataBlobExtra()
{
  LOG_TRACE_FLOW("DataBlobExtra destructor");
  delete itsOut;
  delete itsIn;
}

void DataBlobExtra::pack()
{
  LOG_TRACE_FLOW("DataBlobExtra pack()");
  // Only do something if a createExtraBlock was done.
  // This has the following effect:
  // - an existing extra blob in the buffer is kept.
  // - if the DataHolder writes data that was read before, the blob is kept.
  if (itsCreateDone) {
    if (itsBufOut.size() > 0) {
      // First end the data.
      itsOut->putEnd();
    }
    itsCreateDone = false;
    // Append the extra block to the main block and clear it thereafter.
    itsDH->putExtra (itsBufOut.getBuffer(), itsBufOut.size());
  }
}

BlobOStream& DataBlobExtra::createBlock()
{
  if (itsOut == 0) {
    itsOut = new BlobOStream(itsBufOut);
  }
  itsOut->clear();
  itsBufOut.clear();
  itsOut->putStart (itsName, itsVersion);
  itsCreateDone = true;
  itsLastDone   = 1;
  return *itsOut;
}

void DataBlobExtra::clearBlock()
{
  itsBufOut.clear();
  itsCreateDone = true;
}

void DataBlobExtra::openBlock (const BlobString& data)
{
  clearOut();
  BlobIBufChar bibc(data.data(), data.size());
  itsBufIn = itsDH->dataFieldSet().getExtraBlob (bibc);
  itsLastDone = 2;
}

BlobIStream& DataBlobExtra::getBlock (bool& found, int& version)
{
  found = false;
  if (itsLastDone > 0) {
    if (itsLastDone == 1) {
      // A create is done as the last operation; make an input buffer of it.
      found = true;
      itsBufIn = BlobIBufChar (itsBufOut.getBuffer(), itsBufOut.size());
    }
    if (itsIn == 0) {
      itsIn = new BlobIStream(itsBufIn);
    }
    if (itsBufIn.size() > 0) {
      // Make sure the input buffer is positioned at the beginning.
      found = true;
      itsBufIn.setPos (0);
      itsIn->clear();
      version = itsIn->getStart (itsName);
    }
  }
  return *itsIn;
}

BlobIStream& DataBlobExtra::getBlock()
{
  bool found;
  int version;
  BlobIStream& bis = getBlock (found, version);
  ASSERT (found);
  return bis;
}


} // namespace LOFAR
