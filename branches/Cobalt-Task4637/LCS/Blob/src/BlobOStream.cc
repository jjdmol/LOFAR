//# BlobOStream.cc: Output stream for a blob
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobHeader.h>
#include <Common/DataConvert.h>
#include <Blob/BlobException.h>
#include <Common/LofarLogger.h>
#include <iostream>

namespace LOFAR {

BlobOStream::BlobOStream (BlobOBuffer& bb)
: itsCurLength (0),
  itsLevel     (0),
  itsStream    (&bb)
{
  itsSeekable = (bb.tellPos() != -1);
}

BlobOStream::~BlobOStream()
{
  // We could check that isLevel==0, but in case this destructor is called
  // by an exception, it gives another exception causing an abort.
  // So it should only be done if it is known that the destructor is called
  // in the normal way, but I don't know how to test that.
  // Maybe uncaught_exception is the way to go.
  /////  ASSERT (itsLevel == 0);
}

// putStart starts writing an object.
// It puts the object type and version and reserves space for the length.
// It increases the level for each object to hold the length.
uint BlobOStream::doPutStart (const char* type, uint nrc, int version)
{
  BlobHeader hdr(version, itsLevel);
  ASSERT (nrc < 256);
  hdr.itsNameLength = nrc;
  itsObjLen.push (itsCurLength);         // length of outer blob
  itsObjPtr.push (tellPos()+hdr.lengthOffset()); // remember where to put len
  itsCurLength = 0;                      // initialize object length
  // Need to increment here, otherwise putBuf gives an exception.
  itsLevel++;
  // Put header plus objecttype.
  putBuf (&hdr, sizeof(hdr));
  if (nrc > 0) {
    putBuf (type, nrc);
  }
  return itsLevel;
}

// putend ends putting an object. It decreases the level and writes
// the object length if the file is seekable.
uint64 BlobOStream::putEnd()
{
  ASSERT (itsLevel > 0);
  uint32 eob = BlobHeader::eobMagicValue();
  *this << eob;                        // write end-of-blob
  uint64 len = itsCurLength;           // length of this object
  itsCurLength = itsObjLen.top();      // length of parent object
  int64 pos    = itsObjPtr.top();
  itsObjLen.pop();
  itsObjPtr.pop();
  if (itsSeekable) {
    int64 curpos = tellPos();
    itsStream->setPos (pos);
    itsStream->put (&len, sizeof(len));
    itsStream->setPos (curpos);
  }
  itsLevel--;
  if (itsLevel > 0) {
    itsCurLength += len;               // add length to parent object
  }
  return len;
}

void BlobOStream::putBuf (const void* buf, uint64 sz)
{
  checkPut();
  uint64 sz1 = itsStream->put (static_cast<const char*>(buf), sz);
  if (sz1 != sz) {
    THROW (BlobException,
	   "BlobOStream::putBuf - " << sz << " bytes asked, but only "
	   << sz1 << " could be written (pos=" << tellPos() << ")");
  }
  itsCurLength += sz1;
}

BlobOStream& BlobOStream::operator<< (const bool& var)
{
  char v = (var ? 1 : 0);
  putBuf (&v, 1);
  return *this;
}
BlobOStream& BlobOStream::operator<< (const char& var)
{
  putBuf (&var, 1);
  return *this;
}
BlobOStream& BlobOStream::operator<< (const int8& var)
{
  putBuf (&var, 1);
  return *this;
}
BlobOStream& BlobOStream::operator<< (const uint8& var)
{
  putBuf (&var, 1);
  return *this;
}
BlobOStream& BlobOStream::operator<< (const int16& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const uint16& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const int32& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const uint32& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const int64& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const uint64& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const float& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const double& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const i4complex& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const i16complex& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const u16complex& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const fcomplex& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const dcomplex& var)
{
  putBuf (&var, sizeof(var));
  return *this;
}
BlobOStream& BlobOStream::operator<< (const std::string& var)
{
  operator<< (int64(var.size()));
  putBuf (var.data(), var.size());
  return *this;
}
BlobOStream& BlobOStream::operator<< (const char* var)
{
  int64 sz = strlen(var);
  operator<< (sz);
  putBuf (var, sz);
  return *this;
}

void BlobOStream::put (const bool* values, uint64 nrval)
{
  uchar buf[256];
  while (nrval > 0) {
    uint nr = std::min(nrval, uint64(8*256));
    // Convert to bits and put.
    uint nrb = LOFAR::boolToBit (buf, values, nr);
    putBuf (buf, nrb);
    nrval -= nr;
    values += nr;
  }
}
void BlobOStream::put (const char* values, uint64 nrval)
{
  putBuf (values, nrval);
}
void BlobOStream::put (const int8* values, uint64 nrval)
{
  putBuf (values, nrval);
}
void BlobOStream::put (const uint8* values, uint64 nrval)
{
  putBuf (values, nrval);
}
void BlobOStream::put (const int16* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(int16));
}
void BlobOStream::put (const uint16* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(uint16));
}
void BlobOStream::put (const int32* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(int32));
}
void BlobOStream::put (const uint32* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(uint32));
}
void BlobOStream::put (const int64* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(int64));
}
void BlobOStream::put (const uint64* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(uint64));
}
void BlobOStream::put (const float* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(float));
}
void BlobOStream::put (const double* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(double));
}
void BlobOStream::put (const i4complex* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(i4complex));
}
void BlobOStream::put (const i16complex* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(i16complex));
}
void BlobOStream::put (const u16complex* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(u16complex));
}
void BlobOStream::put (const fcomplex* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(fcomplex));
}
void BlobOStream::put (const dcomplex* values, uint64 nrval)
{
  putBuf (values, nrval*sizeof(dcomplex));
}
void BlobOStream::put (const string* values, uint64 nrval)
{
  for (uint64 i=0; i<nrval; i++) {
    *this << values[i];
  }
}

void BlobOStream::putBoolVec (const std::vector<bool>& values)
{
  uint64 sz = values.size();
  // Convert to bools and put as such.
  bool buf[256];
  uint64 inx=0;
  while (sz > 0) {
    uint nr = std::min(sz, uint64(256));
    for (uint i=0; i<nr; i++) {
      buf[i] = values[inx++];
    }
    put (buf, nr);
    sz -= nr;
  }
}

int64 BlobOStream::setSpace (uint64 nbytes)
{
  checkPut();
  int64 pos = tellPos();
  if (pos == -1) {
    THROW (BlobException,
	   "BlobOStream::setSpace cannot be done; "
	   "its BlobOBuffer is not seekable");
  }
  itsStream->setPos (pos+nbytes);
  itsCurLength += nbytes;
  return pos;
}

uint BlobOStream::align (uint n)
{
  uint nfill = 0;
  if (n > 1) {
    int64 pos = tellPos();
    if (pos > 0) {
      nfill = pos % n;
    }
  }
  if (nfill > 0) {
    char fill=0;
    nfill = n-nfill;
    for (uint i=0; i<nfill; i++) {
      uint sz1 = itsStream->put (&fill, 1);
      if (sz1 != 1) {
	THROW (BlobException,
	       "BlobOStream::align - could not write fill (pos="
	       << tellPos() << ")");
      }
      itsCurLength++;
    }
  }
  return nfill;
}

void BlobOStream::throwPut() const
{
  THROW (BlobException, "BlobOStream: putStart should be done first");
}


} // end namespace
