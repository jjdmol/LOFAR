//#  -*- mode: c++ -*-
//#
//#  Marshalling.h: Macros for packing/unpacking blitz arrays.
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

#ifndef MARSHALLING_H_
#define MARSHALLING_H_

#include <Common/LofarTypes.h>
#include <blitz/array.h>
#include <string.h>
#include <string>

#define MSH_ARRAY_SIZE(array, datatype)					     \
(((array).dimensions()*sizeof(int32)) + ((array).size() * sizeof(datatype)))

#define MSH_PACK_ARRAY(bufptr, offset, array, datatype)					     \
do {											     \
  for (int dim = blitz::firstDim; dim < blitz::firstDim + (array).dimensions(); dim++)	     \
  {											     \
    int32 extent = (array).extent(dim);							     \
    memcpy(((char*)(bufptr)) + (offset), &extent, sizeof(int32));			     \
    offset += sizeof(int32);								     \
  }											     \
											     \
  if ((array).isStorageContiguous())							     \
  {											     \
    memcpy(((char*)(bufptr)) + (offset), (array).data(), (array).size() * sizeof(datatype)); \
    offset += (array).size() * sizeof(datatype);					     \
  }											     \
  else											     \
  {											     \
    LOG_FATAL("array must be contiguous");						     \
    exit(EXIT_FAILURE);									     \
  }											     \
} while (0)

#define MSH_UNPACK_ARRAY(bufptr, offset, array, datatype, dims)			       \
do {										       \
  blitz::TinyVector<int, (dims)> extent;					       \
										       \
  for (int dim = blitz::firstDim; dim < blitz::firstDim + (dims); dim++)	       \
  {										       \
    int32 extenttmp = array.extent(dim);					       \
    memcpy(&extenttmp, ((char*)(bufptr)) + (offset), sizeof(int32));		       \
    offset += sizeof(int32);							       \
    extent(dim - blitz::firstDim) = extenttmp;					       \
  }										       \
										       \
  /* resize the array to the correct size */					       \
  array.resize(extent);								       \
										       \
  memcpy(array.data(), ((char*)(bufptr)) + (offset), array.size() * sizeof(datatype)); \
  offset += array.size() * sizeof(datatype);					       \
} while (0)

#define MSH_STRING_SIZE(stdstring) (sizeof(uint32) + (stdstring).size() * sizeof(char))

#define MSH_PACK_STRING(bufptr, offset, stdstring)					\
do {											\
  /* pack stdstring with length */							\
  uint32 size = (stdstring).size() * sizeof(char);					\
  memcpy(((char*)(bufptr)) + (offset), &size, sizeof(size));				\
  offset += sizeof(size);								\
  memcpy(((char*)(bufptr)) + (offset), (stdstring).c_str(), size * sizeof(char));	\
  offset += size * sizeof(char);							\
} while (0)

#define MSH_UNPACK_STRING(bufptr, offset, stdstring)				\
do {										\
  uint32 size = 0;								\
  memcpy(&size, ((char*)(bufptr)) + (offset), sizeof(size));			\
  offset += sizeof(size);							\
  char stringbuf[size + 1];							\
  memcpy(stringbuf, ((char*)(bufptr)) + (offset), size * sizeof(char));	        \
  stringbuf[size] = '\0';							\
  (stdstring) = string(stringbuf); /* cast to std::string */			\
  offset += size * sizeof(char);						\
} while (0)

#endif /* MARSHALLING_H_ */
