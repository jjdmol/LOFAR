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
#include <Common/Exception.h>

namespace LOFAR {
  
  BlobFieldSet::BlobFieldSet (const std::string& name)
    : itsName          (name),
      itsVersion       (0),
      itsHasFixedShape (true)
  {}

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
      for (uint i=0; i<itsFields.size(); i++) {
	delete itsFields[i];
      }
      itsFields.resize (0);
      itsFields.reserve (that.itsFields.size());
      for (uint i=0; i<that.itsFields.size(); i++) {
	itsFields.push_back (that.itsFields[i]->clone());
      }
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
      throw Exception("BlobFieldBase::add - field" + name + " already exists");
    }
    if (field.getVersion() > itsVersion) {
      itsVersion = field.getVersion();
    }
    itsHasFixedShape &= field.hasFixedShape();
    int inx = itsFields.size();
    itsFields.push_back (field.clone());
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

  void BlobFieldSet::fill (BlobOStream& bs)
  {
    bs.putStart (itsName, itsVersion);
    for (uint i=0; i<itsFields.size(); i++) {
      itsFields[i]->setSpace (bs);
    }
    bs.putEnd();
  }

  void BlobFieldSet::createBlob (BlobOBufChar& buf)
  {
    // First do it on a null buffer to determine the length.
    BlobOBufNull nbuf;
    BlobOStream nbs(nbuf);
    fill (nbs);
    // Now reserve enough space in the buffer.
    // In this way we avoid possibly costly resizes.
    buf.reserve (nbuf.size());
    BlobOStream bs(buf);
    fill (bs);
  }

  void BlobFieldSet::openBlob (BlobIBufChar& buf)
  {
    BlobIStream bs(buf);
    uint version = bs.getStart (itsName);
    if (version > itsVersion) {
      throw (LOFAR::Exception("BlobFieldSet: blob has an unknown version"));
    }
    for (uint i=0; i<itsFields.size(); i++) {
      itsFields[i]->getSpace (bs, version);
    }
    bs.getEnd();
  }

  BlobFieldBase& BlobFieldSet::operator[] (const std::string& name)
  {
    NameMap::iterator iter = itsNameMap.find (name);
    if (iter == itsNameMap.end()) {
      throw Exception("BlobFieldBase: " + name + " is an unknown field");
    }
    return *(itsFields[iter->second]); 
  }

  const BlobFieldBase& BlobFieldSet::operator[] (const std::string& name) const
  {
    NameMap::const_iterator iter = itsNameMap.find (name);
    if (iter == itsNameMap.end()) {
      throw Exception("BlobFieldBase: " + name + " is an unknown field");
    }
    return *(itsFields[iter->second]); 
  }

  bool BlobFieldSet::checkHeader (BlobIBufChar& buf, const char* objectType,
				  int version, uint size)
  {
    BlobHeaderBase* hdr = (BlobHeaderBase*)(buf.getBuffer());
    if (! hdr->checkMagicValue()) {
      throw Exception("BlobFieldSet::checkHeader - magic value mismatch");
    }
    int vrs = hdr->getVersion();
    if (objectType != 0) {
      if (! hdr->checkType (objectType)) {
	throw Exception("BlobFieldSet::checkHeader - object type mismatch");
      }
      if (version >= 0) {
	if (version < vrs) {
	  throw Exception("BlobFieldSet::checkHeader - blob version too high");
	}
      } else {
	if (-version != vrs) {
	  throw Exception("BlobFieldSet::checkHeader - blob version mismatch");
	}
      }
    }
    if (size > 0) {
      if (size != hdr->getLength()) {
	throw Exception("BlobFieldSet::checkHeader - blob size mismatch");
      }
    }
    return hdr->mustConvert();
  }

  void BlobFieldSet::convertData (BlobIBufChar& buf) const
  {
    BlobHeaderBase* hdr = (BlobHeaderBase*)(buf.getBuffer());
    if (hdr->mustConvert()) {
      LOFAR::DataFormat fmt = hdr->getDataFormat();
      for (uint i=0; i<itsFields.size(); i++) {
	itsFields[i]->convertData (buf, fmt);
      }
      hdr->setLocalDataFormat();
    }
  }

} // end namespace
