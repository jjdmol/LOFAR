//#  -*- mode: c++ -*-
//#
//#  XCStatistics.h: Crosslet statistics from the RSP board.
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

#ifndef SHM_XCSTATISTICS_H_
#define SHM_XCSTATISTICS_H_

#include <blitz/array.h>
#include <Common/LofarTypes.h>
#include <complex>

namespace LOFAR {
  namespace SHM_Protocol {

    class XCStatistics
    {
    public:

      /**
       * Constructors for a XCStatistics object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      XCStatistics() { }
	  
      /* Destructor for XCStatistics. */
      virtual ~XCStatistics() {}

      /* get reference to the statistics array */
      blitz::Array<std::complex<double>, 4>& operator()();

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
       * XCStatistics
       * Dimensions are: (N_POL x
       *                  N_POL x
       *                  N_RSPBOARDS * N_BLPS x
       *                  N_RSPBOARDS * N_BLPS)
       */
      blitz::Array<std::complex<double>, 4> m_xstatistics;
    };
  
    inline blitz::Array<std::complex<double>, 4>& XCStatistics::operator()() { return m_xstatistics; }

  };
}; // namespace LOFAR     

#endif /* SHM_XCSTATISTICS_H_ */
