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

#include <complex>
#include <string>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class Clocks
    {
    public:
      /**
       * Constructors for a Clocks object.
       */
      Clocks() : m_state(NOT_MODIFIED) { }
	  
      /* Destructor for Clocks. */
      virtual ~Clocks() {}

      /* get references to the version arrays */
      blitz::Array<uint32, 1>& operator()();

      typedef enum RegisterState {
	INVALID      = 0,
	NOT_MODIFIED = 1,
	MODIFIED     = 2,
	APPLIED      = 3
      };

      /**
       * RegisterState transitions.
       */
      void init(int n)
      {
	m_clocks.resize(n);

	m_state.resize(n);
	m_state = NOT_MODIFIED;
      }

      void modified(int i = -1)
      {
	if (i < 0) m_state = MODIFIED;
	else if (NOT_MODIFIED == m_state(i) || APPLIED == m_state(i)) {
	  m_state(i) = MODIFIED;
	}
      }

      void reset(int i = -1)
      {
	if (i < 0) m_state = NOT_MODIFIED;
	else if (NOT_MODIFIED == m_state(i) || APPLIED == m_state(i)) {
	  m_state(i) = NOT_MODIFIED;
	}
      }

      void applied(int i = -1)
      {
	if (i < 0) m_state = APPLIED;
	else if (MODIFIED == m_state(i)) {
	  m_state(i) = APPLIED;
	}
      }

      void clear(int i = -1)
      {
	if (i < 0) m_state = NOT_MODIFIED;
	else if (APPLIED == m_state(i)) {
	  m_state(i) = NOT_MODIFIED;
	}
      }

      RegisterState getState(int i) { return m_state(i); }

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

      /**
       * Keep track of the state of the registers. This
       * is needed to make sure that a change from the cache
       * propagates into the hardware properly.
       */
      blitz::Array<RegisterState, 1> m_state;
    };

    inline blitz::Array<uint32, 1>& Clocks::operator()() { return m_clocks; }
  };
}; // namespace LOFAR

#endif /* CLOCKS_H_ */
