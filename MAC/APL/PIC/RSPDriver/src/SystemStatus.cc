//#  SystemStatus.h: implementation of the SystemStatus class
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

#include "SystemStatus.h"
#include "MEPHeader.h"
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP_Protocol;
using namespace EPA_Protocol;
using namespace std;
using namespace blitz;

#define PACK_ARRAY(bufptr, offset, array, datatype)					  \
do {										  \
  for (int dim = firstDim; dim < firstDim + array.dimensions(); dim++)		  \
  {										  \
    int32 extent = array.extent(dim);						  \
    memcpy((bufptr) + offset, &extent, sizeof(int32));				  \
    offset += sizeof(int32);							  \
  }										  \
										  \
  if (array.isStorageContiguous())						  \
  {										  \
    memcpy((bufptr) + offset, array.data(), array.size() * sizeof(complex<int16>)); \
    offset += array.size() * sizeof(datatype);					  \
  }										  \
  else										  \
  {										  \
    LOG_FATAL("array must be contiguous");					  \
    exit(EXIT_FAILURE);								  \
  }										  \
} while (0)

#define UNPACK_ARRAY(bufptr, offset, array, datatype)			      \
do {									      \
  TinyVector<int, array.dimensions()> extent;				      \
									      \
  for (int dim = firstDim; dim < firstDim + array.dimensions(); dim++)	      \
  {									      \
    int32 extenttmp = array.extent(dim);				      \
    memcpy(&extenttmp, (bufptr) + offset, sizeof(int32));			      \
    offset += sizeof(int32);						      \
    extent(dim - firstDim) = extenttmp;					      \
  }									      \
									      \
  /* resize the array to the correct size */				      \
  array.resize(extent);							      \
									      \
  memcpy(array.data(), (bufptr) + offset, array.size() * sizeof(complex<int16>)); \
  offset += array.size() * sizeof(datatype);				      \
} while (0)

unsigned int SystemStatus::getSize()
{
  return MEPHeader::RSPSTATUS_SIZE;
}

unsigned int SystemStatus::pack  (void* buffer)
{
  unsigned int offset = 0;
  
  PACK_ARRAY((char*)buffer, offset, m_rsp_status, EPA_Protocol::RSPStatus);

  return offset;
}

unsigned int SystemStatus::unpack(void *buffer)
{
  unsigned int offset = 0;
  
  UNPACK_ARRAY(buffer, offset, m_rsp_status, EPA_Protocol::RSPStatus);

  return offset;
}
