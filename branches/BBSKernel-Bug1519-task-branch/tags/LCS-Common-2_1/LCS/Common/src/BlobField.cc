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

#include <Common/BlobField.h>
#include <Common/BlobArray.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

  template<typename T>
  BlobField<T>::BlobField (uint version)
    : BlobFieldBase (version)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0)
    : BlobFieldBase (version, size0)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   bool fortranOrder)
    : BlobFieldBase (version, size0, size1, fortranOrder)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   uint32 size2, bool fortranOrder)
    : BlobFieldBase (version, size0, size1, size2, fortranOrder)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, uint32 size0, uint32 size1,
			   uint32 size2, uint32 size3, bool fortranOrder)
    : BlobFieldBase (version, size0, size1, size2, size3, fortranOrder)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, const std::vector<uint32>& shape,
			   bool fortranOrder)
    : BlobFieldBase (version, shape, fortranOrder)
  {}

  template<typename T>
  BlobField<T>::BlobField (uint version, const uint32* shape, uint16 ndim,
			   bool fortranOrder)
    : BlobFieldBase (version, shape, ndim, fortranOrder)
  {}

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
      bs.align (std::min(sizeof(T),(size_t) 8));
      setOffset (bs.setSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();                        // array offset
      setOffset (setSpaceBlobArray<T> (bs, useBlobHeader(),
				       getShape(), isFortranOrder()),
		 off);
    }
  }

  // Get the space from the BlobIStream.
  template<typename T>
  void BlobField<T>::getISpace (BlobIStream& bs)
  {
    if (isScalar()) {
      bs.align (std::min(sizeof(T),(size_t) 8));
      setOffset (bs.getSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();     // array offset
      std::vector<uint32> shp;
      setOffset (getSpaceBlobArray<T> (bs, useBlobHeader(),
				       shp, fortranOrder()),
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
  void* BlobField<T>::getOData (const std::type_info& info,
#else
  void* BlobField<T>::getOData (const std::type_info&,
#endif
				BlobOBufChar& buf) const
  {
    DBGASSERT(info == typeid(T));
    return getOData(buf);
  }
  
  template<typename T>
#ifdef ENABLE_DBGASSERT
  const void* BlobField<T>::getIData (const std::type_info& info,
#else
  const void* BlobField<T>::getIData (const std::type_info&,
#endif
				      BlobIBufChar& buf) const
  {
    DBGASSERT(info == typeid(T));
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
