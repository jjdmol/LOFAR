//#  -*- mode: c++ -*-
//#
//#  SubbandSelection.h: subband selection class.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef SUBBANDSELECTION_H_
#define SUBBANDSELECTION_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    /**
     * This class is used to contain the subband selection for each RCU.
     *
     * When used in a SETSUBBANDS event the dimensions of the arrays
     * should be:
     *  - subbands[1][nr_selected_subbands].
     *
     * When used in the Cache the dimensions should be:
     *  - subbands[N_RCUS][N_LOCAL_XLETS + N_BEAMLETS]  (combined crosslet and beamlet selection)
     *
     * The values in the subbands array should be 0 <= value < N_SUBBANDS * EPA_Protocol::N_POL
     */
    class SubbandSelection
    {
    public:

      enum {
	UNDEFINED = 0,
	BEAMLET   = 1,
	XLET      = 2,
      };

      /**
       * Constructors for a SubbandSelection object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      SubbandSelection() : m_type(UNDEFINED) {}
      
	  
      /* Destructor for SubbandSelection. */
      virtual ~SubbandSelection() {}

      /**
       * Return the crosslet or beamlet array.
       */
      blitz::Array<uint16, 3>& crosslets();
      blitz::Array<uint16, 3>& beamlets();
       

      /**
       * Return type of selection.
       */
      int getType() const { return (int)m_type; }

      /**
       * Set the type of the subbands selection.
       * @param type Type of the subband selection, valid values are SubbandSelection::BEAMLET (array should have 1 to N_LOCAL_XLETS + N_BEAMLETS values) or
       * SubbandSelection::XLET (array should have 1 value).
       */
      void setType(int type) { m_type = (uint16)type; }

    public:
      /*@{*/
      /**
       * marshalling methods
       */
	size_t getSize() const;
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
      /*@}*/

    private:
      /**
       * Subband selection array.
       * dim 1 = n_rcus (== 1 on SETSUBBANDS, == count(rcumask) on GETSUBBANDS_ACK)
       * dim 2 = number of planes (16bit = 1plane, 8bit=2planes, 4bit=4planes)
       * dim 3 = n_beamlets (if type == BEAMLET)
       * dim 3 = 1          (if type == XLET)
       */
      blitz::Array<uint16, 3> itsCrosslets;
      blitz::Array<uint16, 3> itsBeamlets;

      uint16 m_type; // type of subband selection (BEAMLET or XLET)
    };
    
    inline blitz::Array<uint16, 3>& SubbandSelection::crosslets() { return itsCrosslets; }
    inline blitz::Array<uint16, 3>& SubbandSelection::beamlets() { return itsBeamlets; }
  };
}; // namespace LOFAR

#endif /* SUBBANDSELECTION_H_ */
