//#  -*- mode: c++ -*-
//#
//#  Statistics.h: Statistics information from the RSP board.
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

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace RSP_Protocol
{
  typedef enum StatsReduction
  {
    SUM = 1,
    REPLACE,
    MEAN,
    MAX,
    MIN,
    PRODUCT,
    MIN_INDEX,
    MAX_INDEX,
  };

  class Statistics
  {
    public:

      //
      // Indexes in the statistics array.
      // Don't change the order of these constants,
      // the code relies on it.
      //
      static const uint8 SUBBAND_MEAN  = 0x00;
      static const uint8 SUBBAND_POWER = 0x01;
      static const uint8 BEAMLET_MEAN  = 0x02;
      static const uint8 BEAMLET_POWER = 0x03;
      static const int   N_STAT_TYPES = BEAMLET_POWER + 1;

      /**
       * Constructors for a Statistics object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      Statistics() { }
	  
      /* Destructor for Statistics. */
      virtual ~Statistics() {}

      /* get reference to the weights array */
      blitz::Array<std::complex<double>, 3>& operator()();

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      /**
       * Statistics
       * First dimension is the number of bits in
       * the rcumask.
       * 
       */
      blitz::Array<std::complex<double>, 3> m_statistics;
  };
  
  inline blitz::Array<std::complex<double>, 3>& Statistics::operator()() { return m_statistics; }

};
     
#endif /* STATISTICS_H_ */
