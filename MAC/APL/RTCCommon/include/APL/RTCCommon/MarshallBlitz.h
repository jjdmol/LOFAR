//#  -*- mode: c++ -*-
//#
//#  MarshallBlitz.h: Macros for packing/unpacking blitz arrays
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef MARSHALLBLITZ_H_
#define MARSHALLBLITZ_H_

#include <Common/LofarTypes.h>
#include <blitz/array.h>
#include <cstring>

// SIZE blitz::array<...>
template<typename T, int N> size_t MSH_size( const blitz::Array<T,N> &array )
{
    return array.dimensions() * sizeof(LOFAR::int32) + array.size() * sizeof(T);
}

// PACK blitz::array<...>
// first copy the dimensions of the array, then the array itself.
template<typename T, int N> void MSH_pack( char *bufptr, size_t &offset, const blitz::Array<T,N> &array )
{
    for (int dim = blitz::firstDim; dim < blitz::firstDim + N; dim++) {
        LOFAR::int32 extent = array.extent(dim);
        memcpy(bufptr + offset, &extent, sizeof(LOFAR::int32));
        offset += sizeof(LOFAR::int32);
    }
  
    if ((array).numElements() > 0) {
        if ((array).isStorageContiguous()) {
            memcpy(bufptr + offset, array.data(), array.size() * sizeof(T));
            offset += array.size() * sizeof(T);
        }
        else {
            LOG_FATAL("array must be contiguous");
            exit(EXIT_FAILURE);
        }
    }
}

// UNPACK blitz::array<...>
template<typename T, int N> void MSH_unpack( const char *bufptr, size_t &offset, blitz::Array<T,N> &array )
{
	blitz::TinyVector<int, N> extent;

	for (int dim = blitz::firstDim; dim < blitz::firstDim + N; dim++) {
		LOFAR::int32 extenttmp = array.extent(dim);
		memcpy(&extenttmp, bufptr + offset, sizeof(LOFAR::int32));
		offset += sizeof(LOFAR::int32);
		extent(dim - blitz::firstDim) = extenttmp;
	}

	/* resize the array to the correct size */
	array.resize(extent);

	memcpy(array.data(), bufptr + offset, array.size() * sizeof(T));
	offset += array.size() * sizeof(T);
}

#endif /* MARSHALLBLITZ_H_ */
