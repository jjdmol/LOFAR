//#  -*- mode: c++ -*-
//#
//#  Clocks.h: FPGA firmware version information from the RSP board.
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

#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <APL/RTCCommon/Marshalling.h>
//#include <APL/RTCCommon/RegisterState.h>

#include <complex>
#include <string>
#include <blitz/array.h>
#include <Common/LofarTypes.h>
#include <iostream>

namespace LOFAR {
  namespace RSP_Protocol {

    class Clocks
    {
    public:
      /**
       * Constructors for a Clocks object.
       */
      Clocks() { }
	  
      /* Destructor for Clocks. */
      virtual ~Clocks() {}

      /* get references to the version arrays */
      blitz::Array<uint32, 1>& operator()();

      /* initialize */
      void init(int n)
      {
	m_clocks.resize(n);
	//m_state.resize(n);
      }

      //RTC::RegisterState& getState() { m_state.print(std::cout); return m_state; }

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
       * Clocks
       *
       * Dimensions of the arrays are:
       *  - m_clocks  [N_RSPBOARDS]
       */
      blitz::Array<uint32, 1> m_clocks;

      //RTC::RegisterState m_state;
    };

    inline blitz::Array<uint32, 1>& Clocks::operator()() { return m_clocks; }
  };
}; // namespace LOFAR

#endif /* CLOCKS_H_ */
