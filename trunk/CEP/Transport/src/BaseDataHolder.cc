//# BaseDataHolder.cc:
//#
//# Copyright (C) 2000
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


#include <Common/lofar_iostream.h>
#include <stdexcept>

#include <Transport/BaseDataHolder.h>
#include <Common/BlobField.h>
#include <Common/BlobStringType.h>
#include <Common/BlobIBufString.h>
#include <Common/Debug.h>


namespace LOFAR
{

BaseDataHolder::BaseDataHolder(const string& name, const string& type)
  : itsDataFields     (type),
    itsData           (0),
    itsDataBlob       (0),
    itsTransporter    (this),
    itsName           (name),
    itsType           (type),
    itsReadConvert    (-1)
{
  initDataFields();
}

BaseDataHolder::BaseDataHolder(const BaseDataHolder& that)
  : itsDataFields     (that.itsDataFields),
    itsData           (0),
    itsDataBlob       (0),
    itsTransporter    (that.itsTransporter, this),
    itsName           (that.itsName),
    itsType           (that.itsType),
    itsReadConvert    (that.itsReadConvert)
{
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
  

BaseDataHolder::~BaseDataHolder()
{
  delete itsData;
  delete itsDataBlob;
}

void BaseDataHolder::init() {
  itsTransporter.init();
  basePreprocess();
}

void BaseDataHolder::basePreprocess()
{
  preprocess();
}

void BaseDataHolder::preprocess()
{}

void BaseDataHolder::basePostprocess()
{
  postprocess();
  // Delete the memory.
  delete itsData;
  itsData = 0;
  delete itsDataBlob;
  itsDataBlob = 0;
  initDataFields();
  // Make sure only the DataPacket is part of the data fields.
  BlobFieldSet fset(itsType);
  fset.add (BlobField<DataPacket>(1));
  itsDataFields = fset;
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
  // If the data block is fixed shape and has version 1, we can simply read.
  if (itsDataFields.hasFixedShape()  &&  itsDataFields.version() == 1) {
    result = itsTransporter.read();
  } else {
    // Otherwise do 2 separate reads (one for header and one for data).
    ////result = itsTransporter.readHeader();
      ////result = itsTransporter.readData();
    }
    // Check the data header in debug mode.
#ifdef ENABLE_DBGASSERT
  BlobIBufChar bibc(itsData->data(), itsData->size());
  itsDataFields.checkHeader (bibc, itsType.c_str(),
			     itsDataFields.version(), 0);
#endif
  // Convert the data (swap bytes) if needed.
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

  return result;
}

void BaseDataHolder::write()
{ 
  itsTransporter.write();
}

bool BaseDataHolder::connectTo(BaseDataHolder& thatDH,
			       const TransportHolder& prototype)
{
  return itsTransporter.connect(itsTransporter, thatDH.getTransporter(), 
				prototype);
}

bool BaseDataHolder::connectFrom(BaseDataHolder& thatDH,
				 const TransportHolder& prototype)
{
  return itsTransporter.connect(thatDH.getTransporter(), itsTransporter, 
				prototype);
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
  return itsTransporter.isValid();
}

void BaseDataHolder::initDataFields()
{
  // Make sure only the DataPacket (version 1) is part of the data fields.
  BlobFieldSet fset(itsType);
  fset.add (BlobField<DataPacket>(1));
  itsDataFields = fset;
}

uint BaseDataHolder::addField (const BlobFieldBase& field)
{
  return itsDataFields.add (field);
}

uint BaseDataHolder::addField (const std::string& fieldName,
			   const BlobFieldBase& field)
{
  return itsDataFields.add (fieldName, field);
}

BlobStringType BaseDataHolder::blobStringType()
{
  if (getTransporter().getTransportHolder()) {
    cdebug(3) << "blobStringType "
	      << getTransporter().getTransportHolder()->getType() << endl;
    return getTransporter().getTransportHolder()->blobStringType();
  }
  return BlobStringType(false);
}

void BaseDataHolder::createDataBlock()
{
  if (itsData) {
    itsData->resize(0);
  } else {
    itsData = new BlobString (blobStringType());
    itsDataBlob = new BlobOBufString (*itsData);
  }
  itsDataFields.createBlob (*itsDataBlob);
  itsDataPacketPtr = itsDataFields[0].getData<DataPacket> (*itsDataBlob);
  fillDataPointers();
}

void BaseDataHolder::openDataBlock()
{
  Assert (itsData);
  BlobIBufString bis(*itsData);
  itsDataFields.openBlob (bis);
  itsDataPacketPtr = itsDataFields[0].getData<DataPacket> (*itsDataBlob);
  fillDataPointers();
}

void BaseDataHolder::fillDataPointers()
{}

} // namespace LOFAR


// Instantiate the template.
#include <Common/BlobField.cc>
template class LOFAR::BlobField<LOFAR::BaseDataHolder::DataPacket>;
