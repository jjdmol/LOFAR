//# BlobFieldSet.cc: Definition all fields in a blob
//#
//# Copyright (C) 2004
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

#include <Common/BlobFieldSet.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufNull.h>
#include <Common/BlobHeader.h>
#include <Common/BlobException.h>

namespace LOFAR {

  INIT_TRACER_CONTEXT(BlobFieldSet, "BlobFieldSet");
  
  BlobFieldSet::BlobFieldSet (const std::string& name)
    : itsName          (name),
      itsVersion       (0),
      itsHasFixedShape (true),
      itsNormalSize    (0)
  {
    LOG_TRACE_FLOW_STR ("Constructed set " << name);
  }

  BlobFieldSet::BlobFieldSet (const BlobFieldSet& that)
  {
    operator= (that);
  }

  BlobFieldSet& BlobFieldSet::operator= (const BlobFieldSet& that)
  {
    if (this != &that) {
      itsName          = that.itsName;
      itsVersion       = that.itsVersion;
      itsHasFixedShape = that.itsHasFixedShape;
      itsNormalSize    = that.itsNormalSize;
      for (uint i=0; i<itsFields.size(); i++) {
	delete itsFields[i];
      }
      itsFields.resize (0);
      itsFields.reserve (that.itsFields.size());
      for (uint i=0; i<that.itsFields.size(); i++) {
	itsFields.push_back (that.itsFields[i]->clone());
      }
      itsNameMap = that.itsNameMap;
    }
    return *this;
  }

  BlobFieldSet::~BlobFieldSet()
  {
    for (uint i=0; i<itsFields.size(); i++) {
      delete itsFields[i];
    }
  }

  int BlobFieldSet::add (const std::string& name, const BlobFieldBase& field)
  {
    NameMap::iterator iter = itsNameMap.find (name);
    if (iter != itsNameMap.end()) {
      THROW (BlobException,
	     "BlobFieldBase::add - field" + name + " already exists");
    }
    int inx = add (field);
    itsNameMap[name] = inx;
    return inx;
  }

  int BlobFieldSet::add (const BlobFieldBase& field)
  {
    if (field.getVersion() > itsVersion) {
      itsVersion = field.getVersion();
    }
    itsHasFixedShape &= field.hasFixedShape();
    int inx = itsFields.size();
    itsFields.push_back (field.clone());
    return inx;
  }

  uint BlobFieldSet::findBlobSize()
  {
    // Create the blob in a null buffer to determine the length.
    BlobOBufNull nbuf;
    BlobOStream nbs(nbuf);
    createBlob (nbs);
    return nbuf.size();
  }

  void BlobFieldSet::createBlob (BlobOStream& bs)
  {
    bs.putStart (itsName, itsVersion);
    for (uint i=0; i<itsFields.size(); i++) {
      itsFields[i]->setSpace (bs);
    }
    itsNormalSize = bs.tellPos();
    bs.putEnd();
  }

  void BlobFieldSet::createBlob (BlobOBufChar& buf)
  {
    buf.clear();
    buf.reserve (findBlobSize());
    BlobOStream bs(buf);
    createBlob (bs);
  }

  void BlobFieldSet::openBlob (BlobIBufChar& buf)
  {
    BlobIStream bs(buf);
    uint version = bs.getStart (itsName);
    if (version > itsVersion) {
      THROW (BlobException, "BlobFieldSet: blob has an unknown version");
    }
    for (uint i=0; i<itsFields.size(); i++) {
      itsFields[i]->getSpace (bs, version);
    }
    // All fields have been read.
    // There might be an extra blob at the end.
    // Skip the possible extra blob, but remember its offset.
    itsNormalSize = bs.tellPos();
    BlobIBufChar extra = getExtraBlob (buf);
    bs.getSpace (extra.size());
    bs.getEnd();
  }

  void BlobFieldSet::putExtraBlob (BlobOBufChar& outbuf,
				   const void* inbuf, uint size)
  {
    if (size > 0) {
      // Check if the input buffer is a blob. Get its length.
      const BlobHeader* hdrin = static_cast<const BlobHeader*>(inbuf);
      ASSERT (hdrin->checkMagicValue());
      ASSERT (hdrin->getLength() == size);
    }
    // Check if the output buffer contains a blob (created by createBlob). 
    // Get and check its length.
    BlobHeader* hdrout = (BlobHeader*)(outbuf.getBuffer());
    ASSERT (hdrout->checkMagicValue());
    uint oldsz = hdrout->getLength();
    ASSERT (outbuf.size() == oldsz);
    // Insert the extra blob in the normal blob.
    // Reserve enough space in the buffer.
    uint32 eob = BlobHeader::eobMagicValue();
    outbuf.resize (itsNormalSize + size + sizeof(eob));
    // Put it (followed by end-of-blob) as the extra blob.
    // Reacquire the buffer pointer, because it might have changed by resize.
    uchar* dbuf = const_cast<uchar*>(outbuf.getBuffer());
    memcpy (dbuf+itsNormalSize, inbuf, size);
    memcpy (dbuf+itsNormalSize+size, &eob, sizeof(eob));
    // Update the total blob length.
    hdrout = (BlobHeader*)(dbuf);
    hdrout->setLength (outbuf.size());
  }

  BlobIBufChar BlobFieldSet::getExtraBlob (BlobIBufChar& inbuf)
  {
    int64 off = itsNormalSize;
    const void* extra = inbuf.getBuffer() + off;
    off += sizeof(BlobHeader::eobMagicValue());
    DBGASSERT (inbuf.size() >= off);
    uint size = inbuf.size() - off;
    BlobIBufChar buf(extra, size);
    if (size > 0) {
      // There is something extra. Check if indeed a blob with correct length.
      const BlobHeader* hdr = static_cast<const BlobHeader*>(extra);
      hdr->checkMagicValue();
      ASSERT (hdr->getLength() == size);
    }
    return buf;
  }

  BlobFieldBase& BlobFieldSet::operator[] (const std::string& name)
  {
    NameMap::iterator iter = itsNameMap.find (name);
    if (iter == itsNameMap.end()) {
      THROW (BlobException, "BlobFieldBase: " + name + " is an unknown field");
    }
    return *(itsFields[iter->second]); 
  }

  const BlobFieldBase& BlobFieldSet::operator[] (const std::string& name) const
  {
    NameMap::const_iterator iter = itsNameMap.find (name);
    if (iter == itsNameMap.end()) {
      THROW (BlobException, "BlobFieldBase: " + name + " is an unknown field");
    }
    return *(itsFields[iter->second]); 
  }

  bool BlobFieldSet::checkHeader (BlobIBufChar& buf, const char* objectType,
				  int version, uint size)
  {
    BlobHeader* hdr = (BlobHeader*)(buf.getBuffer());
    if (! hdr->checkMagicValue()) {
      THROW (BlobException,
	     "BlobFieldSet::checkHeader - magic value mismatch");
    }
    int vrs = hdr->getVersion();
    if (objectType != 0) {
      if (strncmp (objectType, (char*)(buf.getBuffer()) + sizeof(BlobHeader),
		   hdr->getNameLength()) != 0) {
      //      if (std::string(objectType) !=
      //	  std::string((char*)(buf.getBuffer()) + sizeof(BlobHeader),
      //		      hdr->getNameLength())) {
	THROW (BlobException,
	       "BlobFieldSet::checkHeader - object type mismatch");
      }
      if (version >= 0) {
	if (version < vrs) {
	  THROW (BlobException,
		 "BlobFieldSet::checkHeader - blob version too high");
	}
      } else {
	if (-version != vrs) {
	  THROW (BlobException,
		 "BlobFieldSet::checkHeader - blob version mismatch");
	}
      }
    }
    if (size > 0) {
      if (size != hdr->getLength()) {
	THROW (BlobException,
	       "BlobFieldSet::checkHeader - blob size mismatch");
      }
    }
    return hdr->mustConvert();
  }

  void BlobFieldSet::convertData (BlobIBufChar& buf) const
  {
    BlobHeader* hdr = (BlobHeader*)(buf.getBuffer());
    if (hdr->mustConvert()) {
      LOFAR::DataFormat fmt = hdr->getDataFormat();
      for (uint i=0; i<itsFields.size(); i++) {
	itsFields[i]->convertData (buf, fmt);
      }
      hdr->setLocalDataFormat();
    }
  }

} // end namespace
