//#  Beamlet2SubbandMap.h: implementation of the Beamlet2SubbandMap class
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

#include "Beamlet2SubbandMap.h"
#include "Marshalling.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace BS_Protocol;
using namespace std;
using namespace blitz;

unsigned int Beamlet2SubbandMap::getSize()
{
  // 1-dimensional array has 1 int32 for length
  // map is converted to array of uint16 of 2 * map.size() elements
  return sizeof(int32) + (m_beamlet2subband.size() * sizeof(uint16) * 2);
}

unsigned int Beamlet2SubbandMap::pack  (void* buffer)
{
  unsigned int offset = 0;

  m_array.resize(m_beamlet2subband.size() * 2); // resize the array
  m_array = 0;

  // convert map to Blitz array
  map<uint16, uint16>::iterator it;
  int i = 0;
  for (it = m_beamlet2subband.begin(); it != m_beamlet2subband.end(); ++it, i+=2) {
    m_array(i)   = it->first;
    m_array(i+1) = it->second;
  }

  MSH_PACK_ARRAY(buffer, offset, m_array, uint16);

  return offset;
}

unsigned int Beamlet2SubbandMap::unpack(void *buffer)
{
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, m_array, uint16, 1);

  // convert Blitz array to map
  ASSERT(0 == m_array.extent(firstDim) % 2);
  m_beamlet2subband.clear();
  for (int i = 0; i < m_array.extent(firstDim) / 2; i++) {
    m_beamlet2subband[m_array(i*2)] = m_array(i*2+1);
  }

  return offset;
}

bitset<EPA_Protocol::MEPHeader::N_SUBBANDS> Beamlet2SubbandMap::getAsBitset() const
{
  bitset<EPA_Protocol::MEPHeader::N_SUBBANDS> result;

  for (map<uint16, uint16>::const_iterator it = m_beamlet2subband.begin();
       it != m_beamlet2subband.end(); ++it) {
    result.set(it->second);
  }

  return result;
}

