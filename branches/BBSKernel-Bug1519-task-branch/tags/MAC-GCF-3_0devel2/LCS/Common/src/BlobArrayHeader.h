//# BlobArrayHeader.h: Standard array header for a blob
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

#ifndef COMMON_BLOBARRAYHEADER_H
#define COMMON_BLOBARRAYHEADER_H

#include <Common/BlobHeader.h>
#include <Common/TypeNames.h>
#include <Common/Debug.h>

namespace LOFAR {

// An array is a special blob, so it has a special header.
// This class defines a standard header meant for static blobs.
// The dynamic headers obey this standard, so a static array blob
// can be read by the function in BlobArray.h.
// The header is templated on data type and dimensionality.
//
// The following is added to the standard header:
// <ul>
//  <li> The type of the blob is "array<X>" where X is the type used
//    (e.g. float).
//  <li> The order of the axes (Fortran or C style).
//  <li> The dimensionality
//  <li> The length of each axis
// </ul>
// The header is derived from the standard BlobHeader<NAMELENGTH>.
// The default for NAMELENGTH is 19 meaning that the name of the array
// blob cannot exceed 19 characters, unless the NAMELENGTH is explicitly
// given.
// <br>Care is taken that the length of the header is a multiple of 8, so
// if the dimensionality is odd, an extra dummy field of 4 bytes is created.
//
// For example:
// <srcblock>
// struct SomeClass {
//   SomeClass() : itsHeader(false) {}
//   BlobArrayHeader<dcomplex,2> itsHeader;
//   dcomplex      itsData[10][20];
// };
// </srcblock>

template<typename T, uint NDIM, uint NAMELENGTH=19>
class BlobArrayHeader : public BlobHeader<NAMELENGTH>
{

public:
  // Construct for the given array type and ordering.
  // Fortran ordering is minor axiws first.
  // The opposite is C ordering which is major axis first.
  // It fills the blob header with a type name like array<float>.
  explicit BlobArrayHeader (bool fortranOrder)
    : BlobHeader<NAMELENGTH> (LOFAR::typeName((const T**)0).c_str(), 1),
      itsAxisOrder (fortranOrder ? 1:0),
      itsFiller    (0),
      itsNdim      (NDIM)
    {}

  // Set the size of max. 4 axes.
  // If there are more axes, setAxisSize should be used for them.
  void setShape (uint size0, uint size1=0, uint size2=0, uint size3=0)
    {
      if (NDIM > 0) itsSize[0] = size0;
      if (NDIM > 1) itsSize[1] = size1;
      if (NDIM > 2) itsSize[2] = size2;
      if (NDIM > 3) itsSize[3] = size3;
      setTotalLength();
    }

  // Set the size of the given axis.
  void setAxisSize (uint axis, uint size)
    {
      DbgAssert (axis < NDIM);
      itsSize[axis] = size;
      setTotalLength();
    }

  // Get the size of the given axis.
  uint getAxisSize (uint axis) const
    {
      DbgAssert (axis < NDIM);
      return (mustConvert() ?
	      LOFAR::dataConvert(getDataFormat(), itsSize[axis]) :
	      itsSize[axis]);
    }

  // Is the array in Fortran order?
  bool isFortranOrder() const
    { return itsAxisOrder; }

  // Check if the type name is correct.
  bool checkArrayType() const
    { return checkType (LOFAR::typeName((const T**)0).c_str()); }

  // Check if the dimensionality is correct.
  bool checkNdim() const
    { return NDIM == (mustConvert() ?
		      LOFAR::dataConvert(getDataFormat(), itsNdim) :
		      itsNdim); }

private:
  void setTotalLength()
    {
      uint32 sz=1;
      for (uint i=0; i<NDIM; i++) {
	sz *= itsSize[i];
      }
      setLength (sz*sizeof(T) + sizeof(*this) +
		 sizeof(BlobHeaderBase::eobMagicValue()));
    }

  char   itsAxisOrder;
  char   itsFiller;
  uint16 itsNdim;
  uint32 itsSize[NDIM/2*2+1];   // make sure odd #values to get multiple of 8
};

} // end namespace

#endif
