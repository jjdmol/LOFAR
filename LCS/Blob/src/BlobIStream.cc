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


BlobIStream::BlobIStream (BlobIBuffer* bb)
: itsCanGet        (false),
  itsMustConvert   (false),
  itsHasCachedType (false),
  itsCurLength     (0),
  itsLevel         (0),
  itsVersion       (0),
  itsStream        (bb)
{
  itsSeekable = (bb->tellPos() != -1);
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
  BlobHeader<0> hdr("", 0);
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
  itsStream->get (ptr, hdr.itsNameLength);
  if (hdr.itsReservedLength > hdr.itsNameLength) {
    char buf[256];
    itsStream->get (buf, hdr.itsReservedLength - hdr.itsNameLength);
  }
  itsCurLength = hdr.plainSize() + hdr.itsReservedLength;    // length read
  itsHasCachedType = true;
  return itsObjectType;
}

int BlobIStream::getStart (const std::string& type)
{
  // Read the header and check if the type matches.
  AssertMsg (type == getNextType(),
	     "BlobIStream::getStart: found object type " <<
	     getNextType() << ", expected " << type);
  itsCanGet = true;                      // getting is possible now
  itsHasCachedType = false;               // type is not cached anymore
  return itsVersion;
}

uint BlobIStream::getEnd()
{
  Assert (itsLevel > 0);
  uint32 toRead = itsObjTLN.top();
  uint32 len    = itsCurLength;
  itsCurLength  = itsObjLen.top();
  itsObjTLN.pop();
  itsObjLen.pop();
  if (itsLevel > 0) {
    AssertMsg (len == toRead  ||  toRead == 0,
	       "BlobIStream::getEnd: part of object not read");
  }
  if (--itsLevel == 0) {
    itsCanGet = 0;                   // reading not possible anymore
  } else {
    itsCurLength += len;
  }
  return len;
}

BlobIStream& BlobIStream::operator>> (bool& var)
{
  Assert (itsCanGet);
  char v;
  itsStream->get (&v, 1);
  var = v;
  itsCurLength++;
  return *this;
}
BlobIStream& BlobIStream::operator>> (char& var)
{
  Assert (itsCanGet);
  itsStream->get (&var, 1);
  itsCurLength++;
  return *this;
}
BlobIStream& BlobIStream::operator>> (uchar& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), 1);
  itsCurLength++;
  return *this;
}
BlobIStream& BlobIStream::operator>> (int16& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint16& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (int32& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint32& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (int64& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (uint64& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvert (itsDataFormat, var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (float& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, &var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (double& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, &var);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (fcomplex& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, &var, 2);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (dcomplex& var)
{
  Assert (itsCanGet);
  itsStream->get ((char*)(&var), sizeof(var));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, &var, 2);
  }
  itsCurLength += sizeof(var);
  return *this;
}
BlobIStream& BlobIStream::operator>> (std::string& var)
{
  Assert (itsCanGet);
  int32 len;
  operator>> (len);
  var.resize (len);              // resize storage
  char* ptr = &(var[0]);         // get actual string
  itsStream->get (ptr, len);
  itsCurLength += len;
  return *this;
}

void BlobIStream::get (bool* values, uint nrval)
{
  Assert (itsCanGet);
  // Get as chars and convert to bools.
  std::vector<char> val(nrval);
  get (&val[0], nrval);
  for (uint i=0; i<nrval; i++) {
    values[i] = val[i];
  }
}
void BlobIStream::get (char* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get (values, nrval);
  itsCurLength += nrval;
}
void BlobIStream::get (uchar* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval);
  itsCurLength += nrval;
}
void BlobIStream::get (int16* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(int16));
  if (itsMustConvert) {
    LOFAR::dataConvert16 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(int16);
}
void BlobIStream::get (uint16* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(uint16));
  if (itsMustConvert) {
    LOFAR::dataConvert16 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(uint16);
}
void BlobIStream::get (int32* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(int32));
  if (itsMustConvert) {
    LOFAR::dataConvert32 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(int32);
}
void BlobIStream::get (uint32* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(uint32));
  if (itsMustConvert) {
    LOFAR::dataConvert32 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(uint32);
}
void BlobIStream::get (int64* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(int64));
  if (itsMustConvert) {
    LOFAR::dataConvert64 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(int64);
}
void BlobIStream::get (uint64* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(uint64));
  if (itsMustConvert) {
    LOFAR::dataConvert64 (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(uint64);
}
void BlobIStream::get (float* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(float));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(float);
}
void BlobIStream::get (double* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(double));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, values, nrval);
  }
  itsCurLength += nrval*sizeof(double);
}
void BlobIStream::get (fcomplex* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(fcomplex));
  if (itsMustConvert) {
    LOFAR::dataConvertFloat (itsDataFormat, values, 2*nrval);
  }
  itsCurLength += nrval*sizeof(fcomplex);
}
void BlobIStream::get (dcomplex* values, uint nrval)
{
  Assert (itsCanGet);
  itsStream->get ((char*)values, nrval*sizeof(dcomplex));
  if (itsMustConvert) {
    LOFAR::dataConvertDouble (itsDataFormat, values, 2*nrval);
  }
  itsCurLength += nrval*sizeof(dcomplex);
}
void BlobIStream::get (string* values, uint nrval)
{
  Assert (itsCanGet);
  for (uint i=0; i<nrval; i++) {
    *this >> values[i];
  }
}

void BlobIStream::get (std::vector<bool>& values)
{
  // Get as chars and convert to bools.
  std::vector<char> val;
  get (val);
  values.resize (val.size());
  for (uint i=0; i<values.size(); i++) {
    values[i] = val[i];
  }
}

int64 BlobIStream::skip (uint nbytes)
{
  Assert (itsCanGet);
  int64 pos = tellPos();
  AssertMsg (pos != -1, "BlobIStream::skip cannot be done; "
	     "its BlobIBuffer is not seekable");
  itsStream->setPos (pos+nbytes);
  itsCurLength += nbytes;
  return pos;
}
