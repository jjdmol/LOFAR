//# BlobIStream.h: Input stream for a blob
//#
//# Copyright (C) 2003
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

#include <Common/BlobIStream.h>
#include <Common/BlobHeader.h>
#include <Common/DataConvert.h>

namespace LOFAR {

BlobIStream::BlobIStream (BlobIBuffer& bb)
: itsMustConvert   (false),
  itsHasCachedType (false),
  itsCurLength     (0),
  itsLevel         (0),
  itsVersion       (0),
  itsStream        (&bb)
{
  itsSeekable = (bb.tellPos() != -1);
}

BlobIStream::~BlobIStream()
{
  // We could check that isLevel==0, but in case this destructor is called
  // by an exception, it gives another exception causing an abort.
  // So it should only be done if it is known that the destructor is called
  // in the normal way, but I don't know how to test that.
  // Maybe uncaught_exception is the way to go.
  /////  Assert (itsLevel == 0);
}

// getNextType gets the object type of the next piece of
// information to read. It can only be used if a file has been
// opened and if no put is in operation.
// It checks if it finds the correct magic value preceeding
// the object type.
const std::string& BlobIStream::getNextType()
{
  // Return current type if already cached.
  if (itsHasCachedType) {
    return itsObjectType;
  }
  // Read header and check the magic value and the level.
  BlobHeaderBase hdr;
  itsStream->get ((char*)(&hdr), hdr.plainSize());
  Assert (hdr.checkMagicValue());
  Assert (itsLevel == hdr.itsLevel);
  // Determine if data has to be converted (in case data format mismatches).
  if (itsLevel == 0) {
    itsMustConvert = hdr.mustConvert();
    itsDataFormat  = LOFAR::DataFormat(hdr.itsDataFormat);
  } else {
    Assert (hdr.itsDataFormat == itsDataFormat);
  }
  // Keep the current length read.
  itsLevel++;
  itsObjLen.push (itsCurLength);
  itsObjTLN.push (hdr.getLength());
  itsVersion = hdr.getVersion();
  itsObjectType.resize (hdr.itsNameLength); // resize string adding trailing 0
  char* ptr = &(itsObjectType[0]);
  itsCurLength = hdr.plainSize();    // length read
  getBuf (ptr, hdr.itsNameLength);
  if (hdr.itsReservedLength > hdr.itsNameLength) {
    char buf[256];
    getBuf (buf, hdr.itsReservedLength - hdr.itsNameLength);
  }
  itsHasCachedType = true;
  return itsObjectType;
}

int BlobIStream::getStart (const std::string& type)
{
  // Read the header and check if the type matches.
  AssertMsg (type == getNextType(),
	     "BlobIStream::getStart: found object type " <<
	     getNextType() << ", expected " << type);
  itsHasCachedType = false;               // type is not cached anymore
  return itsVersion;
}

uint BlobIStream::getEnd()
{
  Assert (itsLevel > 0);
  uint32 eob;
  *this >> eob;
  AssertMsg (eob == BlobHeaderBase::eobMagicValue(),
	     "BlobIStream::getEnd - no end-of-blob value found");
  uint32 toRead = itsObjTLN.top();
  uint32 len    = itsCurLength;
  itsCurLength  = itsObjLen.top();
  itsObjTLN.pop();
  itsObjLen.pop();
  if (itsLevel > 0) {
    AssertMsg (len == toRead  ||  toRead == 0,
	       "BlobIStream::getEnd: part of object not read");
  }
  if (--itsLevel > 0) {
    itsCurLength += len;
  }
  return len;
}

void BlobIStream::getBuf (void* buf, uint sz)
{
  checkGet();
  uint sz1 = itsStream->get (static_cast<char*>(buf), sz);
  AssertMsg (sz1 == sz,
	     "BlobIStream::getBuf - " << sz << " bytes asked, but only "
	     << sz1 << " could be read (pos=" << tellPos() << ")");
  itsCurLength += sz1;
}

BlobIStream& BlobIStream::operator>> (bool& var)
{
  char v;
  getBuf (&v, 1);
  var = v;
  return *this;
}
BlobIStream& BlobIStream::operator>> (char& var)
{
  getBuf (&var, 1);
  return *this;
}
BlobIStream& BlobIStream::operator>> (uchar& var)
{
  getBuf (&var, 1);
  return *this;
}
BlobIStream& BlobIStream::operator>> (int16& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint16& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (int32& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint32& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (int64& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint64& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    var = LOFAR::dataConvert (itsDataFormat, var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (float& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, &var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (double& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, &var);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (fcomplex& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, &var, 2);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (dcomplex& var)
{
  getBuf (&var, sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, &var, 2);
  }
  return *this;
}
BlobIStream& BlobIStream::operator>> (std::string& var)
{
  int32 len;
  operator>> (len);
  var.resize (len);              // resize storage
  char* ptr = &(var[0]);         // get actual string
  getBuf (ptr, len);
  return *this;
}

void BlobIStream::get (bool* values, uint nrval)
{
  uchar buf[256];
  while (nrval > 0) {
    uint nr = std::min(nrval, 8*256u);
    // Get and convert bits to bools.
    int nrb = (nr+7)/8;
    getBuf (buf, nrb);
    LOFAR::bitToBool (values, buf, nr);
    nrval -= nr;
    values += nr;
  }
}
void BlobIStream::get (char* values, uint nrval)
{
  getBuf (values, nrval);
}
void BlobIStream::get (uchar* values, uint nrval)
{
  getBuf (values, nrval);
}
void BlobIStream::get (int16* values, uint nrval)
{
  getBuf (values, nrval*sizeof(int16));
  if (itsMustConvert) {
    LOFAR::dataConvert16 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (uint16* values, uint nrval)
{
  getBuf (values, nrval*sizeof(uint16));
  if (itsMustConvert) {
    LOFAR::dataConvert16 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (int32* values, uint nrval)
{
  getBuf (values, nrval*sizeof(int32));
  if (itsMustConvert) {
    LOFAR::dataConvert32 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (uint32* values, uint nrval)
{
  getBuf (values, nrval*sizeof(uint32));
  if (itsMustConvert) {
    LOFAR::dataConvert32 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (int64* values, uint nrval)
{
  getBuf (values, nrval*sizeof(int64));
  if (itsMustConvert) {
    LOFAR::dataConvert64 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (uint64* values, uint nrval)
{
  getBuf (values, nrval*sizeof(uint64));
  if (itsMustConvert) {
    LOFAR::dataConvert64 (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (float* values, uint nrval)
{
  getBuf (values, nrval*sizeof(float));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (double* values, uint nrval)
{
  getBuf (values, nrval*sizeof(double));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, values, nrval);
  }
}
void BlobIStream::get (fcomplex* values, uint nrval)
{
  getBuf (values, nrval*sizeof(fcomplex));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, values, 2*nrval);
  }
}
void BlobIStream::get (dcomplex* values, uint nrval)
{
  getBuf (values, nrval*sizeof(dcomplex));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, values, 2*nrval);
  }
}
void BlobIStream::get (string* values, uint nrval)
{
  for (uint i=0; i<nrval; i++) {
    *this >> values[i];
  }
}

void BlobIStream::getBoolVec (std::vector<bool>& values, uint sz)
{
  values.resize (sz);
  bool buf[256];
  uint inx=0;
  while (sz > 0) {
    uint nr = std::min(sz, 256u);
    // Get and convert bools to vector.
    get (buf, nr);
    for (uint i=0; i<nr; i++) {
      values[inx++] = buf[i];
    }
    sz -= nr;
  }
}

int64 BlobIStream::getSpace (uint nbytes)
{
  checkGet();
  int64 pos = tellPos();
  AssertMsg (pos != -1, "BlobIStream::getSpace cannot be done; "
	     "its BlobIBuffer is not seekable");
  itsStream->setPos (pos+nbytes);
  itsCurLength += nbytes;
  return pos;
}

uint BlobIStream::align (uint n)
{
  uint nfill = 0;
  if (n > 1) {
    int64 pos = tellPos();
    if (pos > 0) {
      nfill = pos % n;
    }
  }
  if (nfill > 0) {
    char fill;
    nfill = n-nfill;
    for (uint i=0; i<nfill; i++) {
      uint sz1 = itsStream->get (&fill, 1);
      AssertMsg (sz1 == 1,
		 "BlobIStream::align - could not read fill (pos="
		 << tellPos() << ")");
      itsCurLength++;
    }
  }
  return nfill;
}

void BlobIStream::throwGet() const
{
  throw Exception("BlobIStream: getStart should be done first");
}


} // end namespace
