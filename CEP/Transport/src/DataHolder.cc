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


#include <Transport/DataHolder.h>
#include <Transport/DataBlobExtra.h>
#include <Common/BlobField.h>
#include <Common/BlobStringType.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobIBufString.h>
#include <Common/Debug.h>

namespace LOFAR
{

DataHolder::DataHolder(const string& name, const string& type)
  : itsDataFields     (type),
    itsData           (0),
    itsDataBlob       (0),
    itsTransporter    (this),
    itsMaxDataSize    (-1),
    itsIsAddMax       (false),
    itsName           (name),
    itsType           (type),
    itsReadConvert    (-1),
    itsExtraPtr       (0)
{
  initDataFields();
}

DataHolder::DataHolder(const DataHolder& that)
  : itsDataFields     (that.itsType),
    itsData           (0),
    itsDataBlob       (0),
    itsTransporter    (that.itsTransporter, this),
    itsMaxDataSize    (that.itsMaxDataSize),
    itsIsAddMax       (that.itsIsAddMax),
    itsName           (that.itsName),
    itsType           (that.itsType),
    itsReadConvert    (that.itsReadConvert),
    itsExtraPtr       (0)
{
  initDataFields();
  if (that.itsExtraPtr != 0) {
    itsExtraPtr = new DataBlobExtra (that.itsExtraPtr->getName(),
				     that.itsExtraPtr->getVersion(),
				     this);
  }
  // Copying is always done before preprocess is called, so there is
  // no need to copy the data buffers.
  // Note that also the copy constructors of all derived DH classes
  // don't copy data.
  ///  if (that.itsData != 0) {
  ///  itsData = new BlobString(blobStringType());
  ///  itsDataBlob = new BlobOBufString(*itsData);
  ///  itsData->resize (that.itsData->size());
  ///  memcpy (itsData->data(), that.itsData->data(), that.itsData->size());
  ///  }
}
  

DataHolder::~DataHolder()
{
  delete itsData;
  delete itsDataBlob;
  delete itsExtraPtr;
}

void DataHolder::setExtraBlob (const string& name, int version)
{
  delete itsExtraPtr;
  itsExtraPtr = 0;
  itsExtraPtr = new DataBlobExtra (name, version, this);
}

void DataHolder::init() {
  basePreprocess();
}

void DataHolder::basePreprocess()
{
  itsTransporter.init();
  preprocess();
}

void DataHolder::preprocess()
{}

void DataHolder::basePostprocess()
{
  postprocess();
  // Delete the memory.
  delete itsData;
  itsData = 0;
  delete itsDataBlob;
  itsDataBlob = 0;
  delete itsExtraPtr;
  itsExtraPtr = 0;
  // Make sure only the DataPacket is part of the data fields.
  initDataFields();
}

void DataHolder::postprocess()
{}


void DataHolder::dump() const
{
  TRACER2("DataHolder dump: " << itsType << ' ' << itsName);
}

bool DataHolder::read()
{
  bool result = false;
  // If the data block is fixed shape and has version 1, we can simply read.
  bool fixedSized = itsDataFields.hasFixedShape()  &&
                    itsDataFields.version() == 1  &&
                    itsExtraPtr == 0;
  result = itsTransporter.read (fixedSized);
  if (result) {
    // Check and convert data if needed.
    // It also handles variable sized data blobs.
    handleDataRead();
  }
  return result;
}

void DataHolder::handleDataRead()
{
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
}

void DataHolder::write()
{
  // If there might be extra data, we have to write it.
  writeExtra();
  // Let the transporter write all data.
  // Determine if the block is fixed sized.
  bool fixedSized = itsDataFields.hasFixedShape()  &&
                    itsDataFields.version() == 1  &&
                    itsExtraPtr == 0;
  itsTransporter.write (fixedSized);
}

bool DataHolder::connectTo (DataHolder& thatDH,
			    const TransportHolder& prototype,
			    bool blockingComm)
{
  return itsTransporter.connect (thatDH.getTransporter(), 
				 prototype, blockingComm);
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

bool DataHolder::isValid() const
{
  return itsTransporter.isValid();
}

void DataHolder::initDataFields()
{
  // Make sure only the DataPacket (version 1) is part of the data fields.
  BlobFieldSet fset(itsType);
  fset.add (BlobField<DataPacket>(1));
  itsDataFields = fset;
}

uint DataHolder::addField (const BlobFieldBase& field)
{
  return itsDataFields.add (field);
}

uint DataHolder::addField (const std::string& fieldName,
			   const BlobFieldBase& field)
{
  return itsDataFields.add (fieldName, field);
}

BlobStringType DataHolder::blobStringType()
{
  if (getTransporter().getTransportHolder()) {
    cdebug(3) << "blobStringType "
	      << getTransporter().getTransportHolder()->getType() << endl;
    return getTransporter().getTransportHolder()->blobStringType();
  }
  return BlobStringType(false);
}

void DataHolder::createDataBlock()
{
  // Allocate buffer if needed. Otherwise clear it.
  if (!itsData) {
    // If no max size set, see if the data can grow.
    if (itsMaxDataSize < 0) {
      itsMaxDataSize = 0;
      if (getTransporter().getTransportHolder())
      {
	itsIsAddMax = !getTransporter().getTransportHolder()->canDataGrow();
      }
    }
    // If no or an additive maximum, determine the blob length.
    int  initsz    = itsMaxDataSize;
    bool canExpand = false;
    if (itsMaxDataSize == 0  ||  itsIsAddMax) {
      initsz += itsDataFields.findBlobSize();
      canExpand = !itsIsAddMax;
    }
    itsData = new BlobString (blobStringType(), initsz, canExpand);
    itsDataBlob = new BlobOBufString (*itsData);
  }
  itsDataFields.createBlob (*itsDataBlob);
  fillAllDataPointers();
  // Clear extra output buffer if there.
  if (itsExtraPtr) {
    itsExtraPtr->clearOut();
  }
}

void DataHolder::openDataBlock()
{
  Assert (itsData);
  BlobIBufString bis(*itsData);
  itsDataFields.openBlob (bis);
  fillAllDataPointers();
}

void DataHolder::resizeBuffer (uint newSize)
{
  char* oldPtr = itsData->data();
  itsDataBlob->resize (newSize);
  if (oldPtr != itsData->data()) {
    fillAllDataPointers();
  }
}

BlobOStream& DataHolder::createExtraBlob()
{
  Assert (itsExtraPtr != 0);
  return itsExtraPtr->createBlock();
}

void DataHolder::clearExtraBlob()
{
  Assert (itsExtraPtr != 0);
  return itsExtraPtr->clearBlock();
}

BlobIStream& DataHolder::openExtraBlob (bool& found, int& version)
{
  Assert (itsExtraPtr != 0);
  return itsExtraPtr->openBlock (found, version, *itsData);
}

void DataHolder::writeExtra ()
{
  if (itsExtraPtr) {
    itsExtraPtr->write();
  }
}

void DataHolder::putExtra (const void* data, uint size)
{
  char* oldPtr = itsData->data();
  itsDataFields.putExtraBlob (*itsDataBlob, data, size);
  if (oldPtr != itsData->data()) {
    fillAllDataPointers();
  }
}

int DataHolder::getMaxDataSize() const
{
  if (itsMaxDataSize == 0  ||  itsData == 0) {
    return itsMaxDataSize;
  }
  return itsData->size();
}

void DataHolder::fillDataPointers()
{}

} // namespace LOFAR


// Instantiate the template.
#include <Common/BlobField.cc>
template class LOFAR::BlobField<LOFAR::DataHolder::DataPacket>;
