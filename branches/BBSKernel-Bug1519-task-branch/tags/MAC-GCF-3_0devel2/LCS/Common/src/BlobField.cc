//# BlobField.cc: Definition of a field in a blob
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
#include <Common/Debug.h>

namespace LOFAR {

  BlobFieldBase::BlobFieldBase (uint version)
    : itsOffset        (-1),
      itsArrayOffset   (0),
      itsVersion       (version),
      itsNelem         (1),
      itsFortranOrder  (true),
      itsIsScalar      (true),
      itsUseHeader     (false),
      itsHasFixedShape (true)
  {}

  BlobFieldBase::BlobFieldBase (uint version, uint32 size0)
    : itsVersion      (version),
      itsNelem        (size0),
      itsFortranOrder (true)
  {
    itsShapeDef.resize (1);
    itsShapeDef[0] = size0;
    init();
  }

  BlobFieldBase::BlobFieldBase (uint version, uint32 size0, uint32 size1,
				bool fortranOrder)
    : itsVersion      (version),
      itsFortranOrder (fortranOrder)
  {
    itsShapeDef.resize (2);
    itsShapeDef[0] = size0;
    itsShapeDef[1] = size1;
    init();
  }
  BlobFieldBase::BlobFieldBase (uint version, uint32 size0, uint32 size1,
				uint32 size2, bool fortranOrder)
    : itsVersion      (version),
      itsFortranOrder (fortranOrder)
  {
    itsShapeDef.resize (3);
    itsShapeDef[0] = size0;
    itsShapeDef[1] = size1;
    itsShapeDef[2] = size2;
    init();
  }
  BlobFieldBase::BlobFieldBase (uint version, uint32 size0, uint32 size1,
				uint32 size2, uint32 size3, bool fortranOrder)
    : itsVersion      (version),
      itsFortranOrder (fortranOrder)
  {
    itsShapeDef.resize (4);
    itsShapeDef[0] = size0;
    itsShapeDef[1] = size1;
    itsShapeDef[2] = size2;
    itsShapeDef[3] = size3;
    init();
  }
  BlobFieldBase::BlobFieldBase (uint version, const std::vector<uint32>& shape,
				bool fortranOrder)
    : itsVersion      (version),
      itsShapeDef     (shape),
      itsFortranOrder (fortranOrder)
  {
    init();
  }
  BlobFieldBase::BlobFieldBase (uint version, const uint32* shape, uint16 ndim,
				bool fortranOrder)
    : itsVersion      (version),
      itsFortranOrder (fortranOrder)
  {
    itsShapeDef.resize (ndim);
    for (uint i=0; i<ndim; i++) {
      itsShapeDef[i] = shape[i];
    }
    init();
  }

  BlobFieldBase::~BlobFieldBase()
  {}

  void BlobFieldBase::init()
  {
    itsOffset      = -1;
    itsArrayOffset = 0;
    itsIsScalar    = false;
    itsUseHeader   = false;
    itsShape       = itsShapeDef;
    fillNelem();
    itsHasFixedShape = (itsShapeDef.size() > 0  &&  itsNelem != 0);
  }

  void BlobFieldBase::setShape (const std::vector<uint32>& shape)
  {
    if (itsIsScalar) {
      throw Exception("BlobField: cannot set shape of a scalar field");
    }
    if (itsShapeDef.size() > 0) {
      if (itsShapeDef.size() != shape.size()) {
	throw Exception("BlobField: cannot change dimensionality of "
			"this array field");
      }
      for (uint i=0; i<shape.size(); i++) {
	if (itsShapeDef[i] != 0  &&  shape[i] != itsShapeDef[i]) {
	  throw Exception("BlobField: cannot change fixed axis size of "
			  "this array field");
	}
      }
    }
    itsShape = shape;
    fillNelem();
  }

  void BlobFieldBase::fillNelem()
  {
    itsNelem = 1;
    for (uint i=0; i<itsShape.size(); i++) {
      itsNelem *= itsShape[i];
    }
  }

  void BlobFieldBase::getSpace (BlobIStream& bs, uint version)
  {
    // The data is only present if the version in the stream is at least
    // as high as the version of this field.
    if (version >= itsVersion) {
      getISpace (bs);
    } else {
      itsOffset = -1;
    }
  }



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
      bs.align (std::min(sizeof(T),8u));
      setOffset (bs.setSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();     // array offset
      setOffset (setSpaceBlobArray<T> (bs, useBlobHeader(),
				       getShape(), isFortranOrder()), off);
    }
  }

  // Get the space from the BlobIStream.
  template<typename T>
  void BlobField<T>::getISpace (BlobIStream& bs)
  {
    if (isScalar()) {
      bs.align (std::min(sizeof(T),8u));
      setOffset (bs.getSpace (sizeof(T)), 0);
    } else {
      int64 off = bs.tellPos();     // array offset
      std::vector<uint32> shp;
      setOffset (getSpaceBlobArray<T> (bs, useBlobHeader(),
				       shp, fortranOrder()), off);
      setShape (shp);
    }
  }

  template<typename T>
  void* BlobField<T>::getOData (BlobOBufChar& buf) const
  {
    if (getOffset() < 0) {
      return 0;
    }
    DbgAssert (getOffset() + getNelem()*sizeof(T) <= buf.size());
    T* data = (T*)(buf.getBuffer() + getOffset());
    return data;
  }

  template<typename T>
  const void* BlobField<T>::getIData (BlobIBufChar& buf) const
  {
    if (getOffset() < 0) {
      return 0;
    }
    DbgAssert (getOffset() + getNelem()*sizeof(T) <= buf.size());
    T* data = (T*)(buf.getBuffer() + getOffset());
    return data;
  }

  template<typename T>
  void* BlobField<T>::getOData (const std::type_info& info,
				BlobOBufChar& buf) const
  {
    DbgAssert(info == typeid(T));
    return getOData(buf);
  }
  
  template<typename T>
  const void* BlobField<T>::getIData (const std::type_info& info,
				      BlobIBufChar& buf) const
  {
    DbgAssert(info == typeid(T));
    return getIData(buf);
  }

  template<typename T>
  void BlobField<T>::convertData (BlobIBufChar& buf,
				  LOFAR::DataFormat fmt) const
  {
    if (getOffset() >= 0) {
      DbgAssert (getOffset() + getNelem()*sizeof(T) <= buf.size());
      T* data = (T*)(buf.getBuffer() + getOffset());
      LOFAR::dataConvert (fmt, data, getNelem());
      if (! isScalar()) {
	LOFAR::convertArrayHeader (fmt, (char*)(buf.getBuffer() +
						getArrayOffset()),
				   useBlobHeader());
      }
    }
  }


  //# Force the instantiation of the templates.
  template class BlobField<char>;
  template class BlobField<uchar>;
  template class BlobField<int16>;
  template class BlobField<uint16>;
  template class BlobField<int32>;
  template class BlobField<uint32>;
  template class BlobField<int64>;
  template class BlobField<uint64>;
  template class BlobField<float>;
  template class BlobField<double>;
  template class BlobField<fcomplex>;
  template class BlobField<dcomplex>;

} // end namespace
