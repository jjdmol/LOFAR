//#  BeamletWeights.h: implementation of the BeamletWeights class
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

#include "BeamletWeights.h"

#include <Common/LofarTypes.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP_Protocol;
using namespace std;
using namespace blitz;

unsigned int BeamletWeights::getSize()
{
  /* NDIM extent values plus the array data itself */
  return
    ((NDIM * sizeof(int32))
     + (m_weights.size() * sizeof(complex<int16>)));
}

unsigned int BeamletWeights::pack  (void* buffer)
{
  char* bufptr = (char*)buffer;
  unsigned int offset = 0;

  for (int dim = firstDim; dim < firstDim + m_weights.dimensions(); dim++)
  {
    int32 extent = m_weights.extent(dim);
    memcpy(bufptr + offset, &extent, sizeof(int32));
    offset += sizeof(int32);
  }

  if (m_weights.isStorageContiguous())
  {
    memcpy(bufptr + offset, m_weights.data(), m_weights.size() * sizeof(complex<int16>));
    offset += m_weights.size() * sizeof(complex<int16>);
  }
  else
  {
    LOG_FATAL("beamlet weights array must be contiguous");
    exit(EXIT_FAILURE);
  }
    
  return offset;
}

unsigned int BeamletWeights::unpack(void *buffer)
{
  char* bufptr = (char*)buffer;
  unsigned int offset = 0;
  TinyVector<int, NDIM> extent;

  for (int dim = firstDim; dim < firstDim + m_weights.dimensions(); dim++)
  {
    int32 extenttmp = m_weights.extent(dim);
    memcpy(&extenttmp, bufptr + offset, sizeof(int32));
    offset += sizeof(int32);
    extent(dim - firstDim) = extenttmp;
  }

  // resize the array to the correct size
  m_weights.resize(extent);

  memcpy(m_weights.data(), bufptr+offset, m_weights.size() * sizeof(complex<int16>));
  offset += m_weights.size() * sizeof(complex<int16>);
    
  return offset;
}
