//#  -*- mode: c++ -*-
//#
//#  SubbandSelection.h: subband selection class.
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
//#  $Id: SubbandSelection.h 22248 2012-10-08 12:34:59Z overeem $

#ifndef SDOSELECTION_H_
#define SDOSELECTION_H_

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
     *  - subbands[N_RCUS][N_SDO_SUBBANDS]  (combined crosslet and beamlet selection)
     *
     * The values in the subbands array should be 0 <= value < (N_SDO_SUBBANDS/2) * EPA_Protocol::N_POL
     */
    class SDOSelection
    {
    public:

      /**
       * Constructors for a SubbandSelection object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      SDOSelection() {}
	  
      /* Destructor for SubbandSelection. */
      virtual ~SDOSelection() {}

      blitz::Array<uint16, 3>& subbands();
       
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
       * dim 1 = n_rcus (== 1 on SETSDO, == count(rcumask) on GETSDO_ACK)
       * dim 2 = number of planes (16bit = 1plane, 8bit=2planes, 4bit=4planes)
       * dim 3 = n_subbands
       */
      blitz::Array<uint16, 3> itsSubbands;
    };
    
    inline blitz::Array<uint16, 3>& SDOSelection::subbands() { return itsSubbands; }
  };
}; // namespace LOFAR

#endif /* SDOSELECTION_H_ */
