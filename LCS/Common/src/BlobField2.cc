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
#include <Common/BlobIStream.h>
#include <Common/BlobException.h>

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
      THROW (BlobException,
	     "BlobField: cannot set shape of a scalar field");
    }
    if (itsShapeDef.size() > 0) {
      if (itsShapeDef.size() != shape.size()) {
	THROW (BlobException, "BlobField: cannot change dimensionality of "
	                      "this array field");
      }
      for (uint i=0; i<shape.size(); i++) {
	if (itsShapeDef[i] != 0  &&  shape[i] != itsShapeDef[i]) {
	  THROW (BlobException, "BlobField: cannot change fixed axis size of "
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

} // end namespace



#include <Common/BlobField.cc>

namespace LOFAR {
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
}
