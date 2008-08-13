//#  -*- mode: c++ -*-
//#
//#  Beamlet2SubbandMap.h: FPGA firmware version information from the RSP board.
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

#ifndef BEAMLET2SUBBANDMAP_H_
#define BEAMLET2SUBBANDMAP_H_

#include <MACIO/Marshalling.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/lofar_bitset.h>

namespace LOFAR {
  namespace BS_Protocol {

    class Beamlet2SubbandMap
    {
    public:
      /**
       * Constructors for a Beamlet2SubbandMap object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      Beamlet2SubbandMap() { }
	  
      /* Destructor for Beamlet2SubbandMap. */
      virtual ~Beamlet2SubbandMap() {}

      /* get references to the version arrays */
      std::map<uint16, uint16>& operator()();

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    public:
      /* other methods */
      bitset<EPA_Protocol::MEPHeader::N_SUBBANDS> getAsBitset() const;

    private:
      /**
       * Beamlet2SubbandMap
       *
       * map beamlet (first) to subband (second).
       */
      std::map<uint16, uint16> m_beamlet2subband;
      
    };

    inline std::map<uint16, uint16>& Beamlet2SubbandMap::operator()() { return m_beamlet2subband; }
  };
}; // namespace LOFAR
#endif /* BEAMLET2SUBBANDMAP_H_ */
