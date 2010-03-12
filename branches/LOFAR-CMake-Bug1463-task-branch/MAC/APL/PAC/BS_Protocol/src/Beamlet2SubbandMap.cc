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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/BS_Protocol/Beamlet2SubbandMap.h>
#include <APL/RTCCommon/MarshallBlitz.h>

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
  /**
   * the map is sent as a blitz array
   */
  blitz::Array<uint16, 1> maparray;
  unsigned int offset = 0;

  maparray.resize(m_beamlet2subband.size() * 2); // resize the array
  maparray = 0;

  // convert map to Blitz array
  map<uint16, uint16>::iterator it;
  int i = 0;
  for (it = m_beamlet2subband.begin(); it != m_beamlet2subband.end(); ++it, i+=2) {
    maparray(i)   = it->first;
    maparray(i+1) = it->second;
  }

  MSH_PACK_ARRAY(buffer, offset, maparray, uint16);

  return offset;
}

unsigned int Beamlet2SubbandMap::unpack(void *buffer)
{
  /**
   * the map is received as a blitz array
   */
  blitz::Array<uint16, 1> maparray;
  unsigned int offset = 0;

  MSH_UNPACK_ARRAY(buffer, offset, maparray, uint16, 1);

  // convert Blitz array to map
  ASSERT(0 == maparray.extent(firstDim) % 2);
  m_beamlet2subband.clear();
  for (int i = 0; i < maparray.extent(firstDim) / 2; i++) {
    m_beamlet2subband[maparray(i*2)] = maparray(i*2+1);
  }

  return offset;
}

//
// returns a bitset in which the bits represent the used subbands
//
bitset<EPA_Protocol::MEPHeader::N_SUBBANDS> Beamlet2SubbandMap::getAsBitset() const
{
  bitset<EPA_Protocol::MEPHeader::N_SUBBANDS> result;

  for (map<uint16, uint16>::const_iterator it = m_beamlet2subband.begin();
       it != m_beamlet2subband.end(); ++it) {
    result.set(it->second);
  }

  return result;
}

#if 0
// print (os)
//
ostream& Beamlet2SubbandMap::print (ostream&	os) const
{
	int	idx = 0;
	int	elements = m_beamlet2subband.size();
	int	MAX_ELEMENTS_PER_LINE	= 31;
	cout << "#elements: " << elements << endl;
	os << "#elements: " << elements << endl;
	return (os);
	map<uint16,uint16>::const_iterator	iter;
	map<uint16,uint16>::const_iterator	end = m_beamlet2subband.end();
	while (idx < elements && idx < 248) {
		if (idx % MAX_ELEMENTS_PER_LINE == 0) {
			if (idx % (2*MAX_ELEMENTS_PER_LINE) == 0) {
				os << endl << formatString("[%d]: ", idx / (2*MAX_ELEMENTS_PER_LINE));
			}
			else {
				os << endl << "     ";
			}
		}
		if ((iter = m_beamlet2subband.find(idx)) != end) {
			os << formatString("%3d ", iter->second);
		}
		else {
			os << "  . ";
		}
		idx++;
	}
	os << endl;

	return (os);
}
#endif
