//#  SubbandSelection.h: implementation of the SubbandSelection class
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

#include "SubbandSelection.h"
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP_Protocol;
using namespace std;
using namespace blitz;

unsigned int SubbandSelection::getSize()
{
  return
    ((2 * sizeof(int32))
     + (m_subbands.size() * sizeof(uint16)));
}

unsigned int SubbandSelection::pack  (void* buffer)
{
  char* bufptr = (char*)buffer;
  unsigned int offset = 0;

  for (int dim = firstDim; dim < firstDim + m_subbands.dimensions(); dim++)
  {
    int32 extent = m_subbands.extent(dim);
    memcpy(bufptr + offset, &extent, sizeof(int32));
    offset += sizeof(int32);
  }

  if (m_subbands.isStorageContiguous())
  {
    memcpy(bufptr + offset, m_subbands.data(), m_subbands.size() * sizeof(uint16));
    offset += m_subbands.size() * sizeof(uint16);
  }
  else
  {
    cerr << "NON-CONTIGUOUS ARRAY STORAGE!" << endl;
  }
    
  return offset;
}

unsigned int SubbandSelection::unpack(void *buffer)
{
  char* bufptr = (char*)buffer;
  unsigned int offset = 0;
  TinyVector<int, 2> extent;

  for (int dim = firstDim; dim < firstDim + m_subbands.dimensions(); dim++)
  {
    int32 extenttmp = m_subbands.extent(dim);
    memcpy(&extenttmp, bufptr + offset, sizeof(int32));
    offset += sizeof(int32);
    extent(dim - firstDim) = extenttmp;
  }

  // resize the array to the correct size
  m_subbands.resize(extent);

  if (m_subbands.isStorageContiguous())
  {
    memcpy(m_subbands.data(), bufptr+offset, m_subbands.size() * sizeof(uint16));
    offset += m_subbands.size() * sizeof(uint16);
  }
  else
  {
    LOG_FATAL("NON-CONTIGUOUS ARRAY STORAGE!");
    exit(EXIT_FAILURE);
  }
    
  return offset;
}

Array<uint16,2>& SubbandSelection::operator()()
{
  return m_subbands;
}

