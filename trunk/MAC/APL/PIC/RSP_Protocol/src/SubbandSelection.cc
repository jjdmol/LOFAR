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
#include "Marshalling.h"

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
    MSH_ARRAY_SIZE(m_nrsubbands, uint16)
    + MSH_ARRAY_SIZE(m_subbands, uint16);
}

unsigned int SubbandSelection::pack(void* buffer)
{
  unsigned int offset = 0;

  MSH_PACK_ARRAY(buffer, offset, m_nrsubbands, uint16);
  MSH_PACK_ARRAY(buffer, offset, m_subbands,   uint16);
  
  return offset;
}

unsigned int SubbandSelection::unpack(void *buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, m_nrsubbands, uint16, 1);
  MSH_UNPACK_ARRAY(buffer, offset, m_subbands,   uint16, 2);

  return offset;
}

Array<uint16,2>& SubbandSelection::operator()()
{
  return m_subbands;
}

Array<uint16,1>& SubbandSelection::nrsubbands()
{
  return m_nrsubbands;
}



