//# BlobOStream.cc: Output stream for a blob
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

#include <Common/BlobOStream.h>
#include <Common/BlobHeader.h>
#include <Common/DataConvert.h>

namespace LOFAR {

BlobOStream::BlobOStream (BlobOBuffer* bb, bool header8)
: itsHeader8   (header8),
  itsCurLength (0),
  itsLevel     (0),
  itsStream    (bb)
{
  itsSeekable = (bb->tellPos() != -1);
}

BlobOStream::~BlobOStream()
{
  // We could check that isLevel==0, but in case this destructor is called
  // by an exception, it gives another exception causing an abort.
  // So it should only be done if it is known that the destructor is called
  // in the normal way, but I don't know how to test that.
  // Maybe uncaught_exception is the way to go.
  /////  Assert (itsLevel == 0);
}

// putStart starts writing an object.
// This is not possible if there is no file, if the file is not opened
// for output or if there is a get in operation.
// If it is the root object, it initializes the dynamic buffers.
// It puts the object type and version and reserves space for the length.
// It increases the level for each object to hold the length.
uint BlobOStream::doPutStart (const char* type, uint nrc, int version)
{
  BlobHeaderBase hdr(version, itsLevel);
  Assert (nrc < 256);
  uint nalign = 0;
  if (itsHeader8) {
    nalign = 7 - (hdr.plainSize() + nrc + 7) % 8;
  }
  hdr.itsReservedLength = nrc+nalign;
  hdr.itsNameLength     = nrc;
  itsObjLen.push (itsCurLength);         // length of outer blob
  itsObjPtr.push (tellPos()+hdr.lengthOffset()); // remember where to put len
  itsCurLength = 0;                      // initialize object length
  putBuf (&hdr, hdr.plainSize());
  putBuf (type, nrc);
  if (nalign > 0) {
    putBuf ("        ", nalign);
  }
  itsLevel++;
  return itsLevel;
}

// putend ends putting an object. It decreases the level and writes
// the object length if the file is seekable.
uint BlobOStream::putEnd()
{
  Assert (itsLevel > 0);
  uint32 len = itsCurLength;           // length of this object
  Assert (itsLevel>0);
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

void BlobOStream::putBuf (const void* buf, uint sz)
{
  uint sz1 = itsStream->put (static_cast<const char*>(buf), sz);
  AssertMsg (sz1 == sz,
	     "BlobOStream::putBuf - " << sz << " bytes asked, but only "
	     << sz1 << " could be written (pos=" << tellPos() << ")");
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
BlobOStream& BlobOStream::operator<< (const uchar& var)
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
  operator<< (int32(var.size()));
  putBuf (var.data(), var.size());
  return *this;
}
BlobOStream& BlobOStream::operator<< (const char* var)
{
  int32 sz = strlen(var);
  operator<< (sz);
  putBuf (var, sz);
  return *this;
}

void BlobOStream::put (const bool* values, uint nrval)
{
  uchar buf[256];
  while (nrval > 0) {
    uint nr = std::min(nrval, 8*256u);
    // Convert to bits and put.
    uint nrb = LOFAR::boolToBit (buf, values, nr);
    putBuf (buf, nrb);
    nrval -= nr;
    values += nr;
  }
}
void BlobOStream::put (const char* values, uint nrval)
{
  putBuf (values, nrval);
}
void BlobOStream::put (const uchar* values, uint nrval)
{
  putBuf (values, nrval);
}
void BlobOStream::put (const int16* values, uint nrval)
{
  putBuf (values, nrval*sizeof(int16));
}
void BlobOStream::put (const uint16* values, uint nrval)
{
  putBuf (values, nrval*sizeof(uint16));
}
void BlobOStream::put (const int32* values, uint nrval)
{
  putBuf (values, nrval*sizeof(int32));
}
void BlobOStream::put (const uint32* values, uint nrval)
{
  putBuf (values, nrval*sizeof(uint32));
}
void BlobOStream::put (const int64* values, uint nrval)
{
  putBuf (values, nrval*sizeof(int64));
}
void BlobOStream::put (const uint64* values, uint nrval)
{
  putBuf (values, nrval*sizeof(uint64));
}
void BlobOStream::put (const float* values, uint nrval)
{
  putBuf (values, nrval*sizeof(float));
}
void BlobOStream::put (const double* values, uint nrval)
{
  putBuf (values, nrval*sizeof(double));
}
void BlobOStream::put (const fcomplex* values, uint nrval)
{
  putBuf (values, nrval*sizeof(fcomplex));
}
void BlobOStream::put (const dcomplex* values, uint nrval)
{
  putBuf (values, nrval*sizeof(dcomplex));
}
void BlobOStream::put (const string* values, uint nrval)
{
  for (uint i=0; i<nrval; i++) {
    *this << values[i];
  }
}

void BlobOStream::put (const std::vector<bool>& values)
{
  // Convert to bools and put as such.
  uint32 sz = values.size();
  operator<< (sz);
  bool buf[256];
  uint inx=0;
  while (sz > 0) {
    uint nr = std::min(sz, 256u);
    for (uint i=0; i<nr; i++) {
      buf[i] = values[inx++];
    }
    put (buf, nr);
    sz -= nr;
  }
}

int64 BlobOStream::setSpace (uint nbytes)
{
  int64 pos = tellPos();
  AssertMsg (pos != -1, "BlobOStream::setSpace cannot be done; "
	     "its BlobOBuffer is not seekable");
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
  for (uint i=0; i<nfill; i++) {
    *this << char(0);
  }
  return nfill;
}

} // end namespace
