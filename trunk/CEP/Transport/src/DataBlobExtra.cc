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


#include <Transport/DataBlobExtra.h>
#include <Transport/DataHolder.h>
#include <Common/BlobField.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/Debug.h>
#include <vector>

namespace LOFAR
{

DataBlobExtra::DataBlobExtra(const string& name, int version, DataHolder* DH)
  : itsOut     (0),
    itsIn      (0),
    itsBufOut  (0),
    itsBufIn   (0, 0),
    itsName    (name),
    itsVersion (version),
    itsDH      (DH)
{}

DataBlobExtra::~DataBlobExtra()
{
  delete itsOut;
  delete itsIn;
}

void DataBlobExtra::write()
{
  // If the user has not started the extra block, it is done here.
  if (itsBufOut.size() == 0) {
    createBlock();
  }
  // First end the data (as started by createExtraBlock).
  itsOut->putEnd();
  // Append the extra block to the main block and clear it thereafter.
  itsDH->putExtra (itsBufOut.getBuffer(), itsBufOut.size());
  itsBufOut.clear();
}

BlobOStream& DataBlobExtra::createBlock()
{
  if (itsOut == 0) {
    itsOut = new BlobOStream(itsBufOut);
  }
  itsOut->clear();
  itsBufOut.clear();
  itsOut->putStart (itsName, itsVersion);
  return *itsOut;
}

BlobIStream& DataBlobExtra::openBlock (int& version)
{
  const BlobString& dataBlock = itsDH->getDataBlock();
  int size = dataBlock.size() - (itsDataPtr - dataBlock.data());
  itsBufIn = BlobIBufChar(itsDataPtr, size);
  if (itsIn == 0) {
    itsIn = new BlobIStream(itsBufIn);
  }
  version = itsIn->getStart (itsName);
  return *itsIn;
}


} // namespace LOFAR
