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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/SubbandSelection.h>
#include <APL/RTCCommon/MarshallBlitz.h>

#include <blitz/array.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RSP_Protocol;

size_t SubbandSelection::getSize() const
{
  cout << itsCrosslets.dimensions() << "; " << itsCrosslets.size() << endl;
  cout << itsBeamlets.dimensions() << "; " << itsBeamlets.size() << endl;
  
  return MSH_size(itsCrosslets)
       + MSH_size(itsBeamlets)
       + sizeof(uint16);
}

size_t SubbandSelection::pack(char* buffer) const
{
  size_t offset = 0;

  MSH_pack(buffer, offset, itsCrosslets);
  MSH_pack(buffer, offset, itsBeamlets);
  memcpy(buffer + offset, &m_type, sizeof(uint16));
  offset += sizeof(uint16);
  
  return offset;
}

size_t SubbandSelection::unpack(const char *buffer)
{
  size_t offset = 0;

  MSH_unpack(buffer, offset, itsCrosslets);
  MSH_unpack(buffer, offset, itsBeamlets);
  memcpy(&m_type, buffer + offset, sizeof(uint16));
  offset += sizeof(uint16);

  return offset;
}



