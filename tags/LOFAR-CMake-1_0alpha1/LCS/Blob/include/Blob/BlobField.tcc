//# BlobField.cc: Templated BlobField classes
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Blob/BlobField.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobOBufChar.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

  template<typename T>
  BlobField<T>::BlobField (uint version)
    : BlobFieldBase (version)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0)
    : BlobFieldBase (version, size0)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   bool fortranOrder)
    : BlobFieldBase (version, size0, size1, fortranOrder)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   uint32 size2, bool fortranOrder)
    : BlobFieldBase (version, size0, size1, size2, fortranOrder)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   uint32 size2, uint32 size3, bool fortranOrder)
    : BlobFieldBase (version, size0, size1, size2, size3, fortranOrder)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, const std::vector<uint32>& shape,
			   bool fortranOrder)
    : BlobFieldBase (version, shape, fortranOrder)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::BlobField (uint version, const uint32* shape, uint16 ndim,
			   bool fortranOrder)
    : BlobFieldBase (version, shape, ndim, fortranOrder)
  {
    setAlignment (p2divider(sizeof(T)));
  }

  template<typename T>
  BlobField<T>::~BlobField()
  {}

  template<typename T>
  BlobFieldBase* BlobField<T>::clone() const
  {
    return new BlobField<T>(*this);
  }

  template<typename T>
  void BlobField<T>::setOSpace (BlobOStream& bs)
  {
    if (isScalar()) {
      bs.align (getAlignment());
      setOffset (bs.setSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();                        // array offset
      setOffset (setSpaceBlobArray<T> (bs, useBlobHeader(),
				       getShape(), isFortranOrder(),
				       getAlignment()),
		 off);
    }
  }

  // Get the space from the BlobIStream.
  template<typename T>
  void BlobField<T>::getISpace (BlobIStream& bs)
  {
    if (isScalar()) {
      bs.align (getAlignment());
      setOffset (bs.getSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();     // array offset
      std::vector<uint32> shp;
      setOffset (getSpaceBlobArray<T> (bs, useBlobHeader(),
				       shp, rwFortranOrder()),
		 off);
      setShape (shp);
    }
  }

  template<typename T>
  void* BlobField<T>::getOData (BlobOBufChar& buf) const
  {
    if (getOffset() < 0) {
      return 0;
    }
    DBGASSERT (getOffset() + getNelem()*sizeof(T) <= buf.size());
    T* data = (T*)(buf.getBuffer() + getOffset());
    return data;
  }

  template<typename T>
  const void* BlobField<T>::getIData (BlobIBufChar& buf) const
  {
    if (getOffset() < 0) {
      return 0;
    }
    DBGASSERT (getOffset() + getNelem()*sizeof(T) <= buf.size());
    T* data = (T*)(buf.getBuffer() + getOffset());
    return data;
  }

  //# The ifdef is needed to avoid compiler warnings about unused.
  template<typename T>
#ifdef ENABLE_DBGASSERT
  void* BlobField<T>::getOData (const std::string& type,
#else
  void* BlobField<T>::getOData (const std::string&,
#endif
				BlobOBufChar& buf) const
  {
    DBGASSERT(type == typeName((T*)0));
    return getOData(buf);
  }
  
  template<typename T>
#ifdef ENABLE_DBGASSERT
  const void* BlobField<T>::getIData (const std::string& type,
#else
  const void* BlobField<T>::getIData (const std::string&,
#endif
				      BlobIBufChar& buf) const
  {
    DBGASSERT(type == typeName((T*)0));
    return getIData(buf);
  }

  template<typename T>
  void BlobField<T>::convertData (BlobIBufChar& buf,
				  LOFAR::DataFormat fmt) const
  {
    if (getOffset() >= 0) {
      DBGASSERT (getOffset() + getNelem()*sizeof(T) <= buf.size());
      T* data = (T*)(buf.getBuffer() + getOffset());
      LOFAR::dataConvert (fmt, data, getNelem());
      if (! isScalar()) {
	LOFAR::convertArrayHeader (fmt, (char*)(buf.getBuffer() +
						getArrayOffset()),
				   useBlobHeader());
      }
    }
  }

} // end namespace
