//# DataHolder.cc:
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

#include <Transport/DataHolder.h>
#include <Transport/DataBlobExtra.h>
#include <Blob/BlobField.h>
#include <Blob/BlobStringType.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufString.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

DataHolder::DataHolder(const string& name, const string& type, int version)
  : itsDataFields     (type),
    itsData           (0),
    itsDataBlob       (0),
    itsBlobType       (0),
    itsMaxDataSize    (-1),
    itsDataCanGrow    (true),
    itsName           (name),
    itsType           (type),
    itsVersion        (version),
    itsReadConvert    (-1),
    itsExtraPtr       (0),
    itsInitialized    (false)
{
  LOG_TRACE_FLOW("DataHolder constructor");
  initDataFields();
}

DataHolder::DataHolder(const DataHolder& that)
  : itsDataFields     (that.itsType),
    itsData           (0),
    itsDataBlob       (0),
    itsBlobType       (0),
    itsMaxDataSize    (that.itsMaxDataSize),
    itsDataCanGrow    (that.itsDataCanGrow),
    itsName           (that.itsName),
    itsType           (that.itsType),
    itsVersion        (that.itsVersion),
    itsReadConvert    (that.itsReadConvert),
    itsExtraPtr       (0),
    itsInitialized    (that.itsInitialized)
{
  LOG_TRACE_FLOW("DataHolder copy constructor");
  initDataFields();
  if (that.itsExtraPtr != 0) {
    itsExtraPtr = new DataBlobExtra (that.itsExtraPtr->getName(),
				     that.itsExtraPtr->getVersion(),
				     this);
  }
  if (that.itsBlobType != 0) {
    itsBlobType = new BlobStringType(*(that.itsBlobType));
  }
  // Note: The data blob is not copied.
}
  

DataHolder::~DataHolder()
{
  LOG_TRACE_FLOW("DataHolder destructor");
  delete itsData;
  delete itsDataBlob;
  delete itsExtraPtr;
  delete itsBlobType;
}

void DataHolder::setExtraBlob (const string& name, int version)
{
  delete itsExtraPtr;
  itsExtraPtr = 0;
  itsExtraPtr = new DataBlobExtra (name, version, this);
}

void DataHolder::init() {
  LOG_TRACE_RTTI("DataHolder init"); 
}

// void DataHolder::basePostprocess()
// {
//   LOG_TRACE_RTTI("DataHolder basePostprocess()");
//   // Delete the memory.
//   delete itsData;
//   itsData = 0;
//   delete itsDataBlob;
//   itsDataBlob = 0;
//   delete itsExtraPtr;
//   itsExtraPtr = 0;
//   // Initialize the data fields.
//   initDataFields();
// }

void DataHolder::pack ()
{
  if (itsExtraPtr) {
    itsExtraPtr->pack();
  }
}

void DataHolder::unpack()
{
  LOG_TRACE_RTTI("DataHolder unpack()");
  DBGASSERT (itsData != 0);
  // Check the data header in debug mode.
#ifdef ENABLE_DBGASSERT
  BlobIBufChar bibc(itsData->data(), itsData->size());
  itsDataFields.checkHeader (bibc, itsType.c_str(),
                             itsDataFields.version(), 0);
#endif
  // If variable sized, reopen the blob.
  if (!(itsDataFields.hasFixedShape()  && itsDataFields.version() == 1)) {
    openDataBlock();
  }
  // Convert the data (swap bytes) if needed.
  // todo: note that for, say, TH_PL it is possible that the database is filled
  // from different sources; in such a case it should be checked for each
  // read if conversion has to be done.
  if (itsReadConvert) {
    BlobIBufChar bib(itsData->data(), itsData->size());
    if (itsReadConvert != 1) {
      // Not known yet if conversion is needed. So determine it.
      // At the same time the header is checked.
      // For the time being, only version 1 sets are supported.
      itsReadConvert = 0;
      if (itsDataFields.checkHeader(bib, itsType.c_str(), 1, 0)) {
        itsReadConvert = 1;
      }
    }
    if (itsReadConvert) {
      itsDataFields.convertData (bib);
    }
  }
  // Open the extra blob (if present).
  if (itsExtraPtr != 0) {
    itsExtraPtr->openBlock (*itsData);
  }
}

void DataHolder::dump() const
{
  LOG_TRACE_FLOW_STR("DataHolder dump: " << itsType << ' ' << itsName);
}

void DataHolder::initDataFields()
{
  BlobFieldSet fset(itsType);
  itsDataFields = fset;
}

uint DataHolder::addField (const BlobFieldBase& field, uint alignment)
{
  return itsDataFields.add (field, alignment);
}

uint DataHolder::addField (const std::string& fieldName,
			   const BlobFieldBase& field, uint alignment)
{
  return itsDataFields.add (fieldName, field, alignment);
}

BlobStringType DataHolder::getBlobStringType()
{
  if (itsInitialized) {
    return BlobStringType(*itsBlobType);		// copy constructor
  }
  return BlobStringType(false);			// constructor
}

void DataHolder::createDataBlock()
{
  // Allocate buffer if needed. Otherwise clear it.
  if (!itsData) {
    // If no max size set, see if the data can grow.
    if (itsMaxDataSize < 0) {
      itsMaxDataSize = 0;
//      itsIsAddMax = !itsDataCanGrow;	// @@@
    }
    // If no or an additive maximum, determine the blob length.
    int64 initsz = itsMaxDataSize;
    if (itsMaxDataSize == 0  ||  !itsDataCanGrow) {
      initsz += itsDataFields.findBlobSize();
    }
    itsData = new BlobString (getBlobStringType(), initsz, itsDataCanGrow,
			      itsDataFields.getAlignment());
    itsDataBlob = new BlobOBufString (*itsData);
  }
  itsDataFields.createBlob (*itsDataBlob);
  fillDataPointers();
  // Clear extra output buffer if there.
  if (itsExtraPtr) {
    itsExtraPtr->clearOut();
  }
  itsInitialized = true;
}

void DataHolder::openDataBlock()
{
  ASSERT (itsData);
  BlobIBufString bis(*itsData);
  itsDataFields.openBlob (bis);
  fillDataPointers();
}

void DataHolder::resizeBuffer (uint64 newSize)
{
  DBGASSERT (itsData != 0);
  char* oldPtr = itsData->data();
  itsDataBlob->resize (newSize);
  if (oldPtr != itsData->data()) {
    fillDataPointers();
  }
}

BlobOStream& DataHolder::createExtraBlob()
{
  ASSERT (itsExtraPtr != 0);
  return itsExtraPtr->createBlock();
}

void DataHolder::clearExtraBlob()
{
  ASSERT (itsExtraPtr != 0);
  return itsExtraPtr->clearBlock();
}

BlobIStream& DataHolder::getExtraBlob()
{
  ASSERT (itsExtraPtr != 0);
  return itsExtraPtr->getBlock();
}

BlobIStream& DataHolder::getExtraBlob (bool& found, int& version)
{
  ASSERT (itsExtraPtr != 0);
  return itsExtraPtr->getBlock (found, version);
}

void DataHolder::putExtra (const void* data, uint64 size)
{
  DBGASSERT (itsData != 0);
  char* oldPtr = itsData->data();
  itsDataFields.putExtraBlob (*itsDataBlob, data, size);
  if (oldPtr != itsData->data()) {
    fillDataPointers();
  }
}

int64 DataHolder::getMaxDataSize() const
{
  if (itsMaxDataSize == 0  ||  itsData == 0) {
    return itsMaxDataSize;
  }
  return itsData->size();
}

void DataHolder::fillDataPointers()
{}

} // namespace LOFAR

