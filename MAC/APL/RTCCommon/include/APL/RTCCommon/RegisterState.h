//#  -*- mode: c++ -*-
//#
//#  RegisterState.h: Macros for packing/unpacking blitz arrays.
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

#ifndef REGISTERSTATE_H_
#define REGISTERSTATE_H_

#include <blitz/array.h>
#include <Common/LofarLogger.h>
#include <iostream>

namespace LOFAR {
  namespace RTC {

    class RegisterState
      {
      public:

	typedef enum State {
	  INVALID      = 0,
	  NOT_MODIFIED = 1,
	  MODIFIED     = 2,
	  APPLIED      = 3,
	  CONFIRMED    = 4,
	};

	explicit RegisterState(State state = NOT_MODIFIED)
	  {
	    m_state.resize(1);
	    m_state = state;
	  }
	virtual ~RegisterState()
	  {
	    m_state.free();
	  }
	

	/**
	 * RegisterState state machine.
	 *
	 *                                modified
	 *   reset ---* NOT_MODIFIED ---------------* MODIFIED  *----- modified
	 *                   *                        /  |
	 *                   |    /------------------/   |applied
	 *            clear  |   /       confirmed       |
	 *                   |  *                        *
	 *               CONFIRMED *----------------- APPLIED
	 *                            confirmed
	 *
	 * Rationale:
	 * When a register is modified, it transitions to the MODIFIED
	 * state. It can only return to NOT_MODIFIED when cleared from
	 * the CONFIRMED state. The CONFIRMED state can only be reached
	 * from the MODIFIED state (with confirmed signal) or via the
	 * APPLIED state.
	 *
	 * A register that is written and read consequently to check that
	 * the data was written correctly would therefor transition from
	 * NOT_MODIFIED, to MODIFIED, the to APPLIED (after the write
	 * has completed) and then to CONFIRMED when the read result
	 * matches what was actually written. Only then can the register
	 * state be cleared to NOT_MODIFIED.
	 */
	void resize(int n)
	  {
	    m_state.resize(n);
	    m_state = NOT_MODIFIED;
	  }

	void modified(int i = -1)
	  {
	    if (i < 0) {
	      for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
		m_state(j) = MODIFIED;
	      }
	      return;
	    }
	    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	    m_state(i) = MODIFIED;
	  }

	void reset(int i = -1)
	  {
	    if (i < 0) {
	      for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
		m_state(j) = NOT_MODIFIED;
	      }
	      return;
	    }
	    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	    m_state(i) = NOT_MODIFIED;
	  }

	void applied(int i = -1)
	  {
	    if (i < 0) {
	      for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
		if (MODIFIED == m_state(j)) {
		  m_state(j) = APPLIED;
		}
	      }
	      return;
	    }
	    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	    if (MODIFIED == m_state(i)) {
	      m_state(i) = APPLIED;
	    }
	  }

	void confirmed(int i = -1)
	  {
	    if (i < 0) {
	      for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
		if (MODIFIED == m_state(j) || APPLIED == m_state(j)) {
		  m_state(j) = CONFIRMED;
		}
	      }
	      return;
	    }
	    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	    if (MODIFIED == m_state(i) || APPLIED == m_state(i)) {
	      m_state(i) = CONFIRMED;
	    }
	  }

	void clear(int i = -1)
	  {
	    if (i < 0) {
	      for (int j = 0; j < m_state.extent(blitz::firstDim); j++) {
		if (CONFIRMED == m_state(j)) {
		  m_state(j) = NOT_MODIFIED;
		}
	      }
	      return;
	    }
	    ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	    if (CONFIRMED == m_state(i)) {
	      m_state(i) = NOT_MODIFIED;
	    }
	  }

	State get(int i) {
	  ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	  return m_state(i);
	}

	void print(std::ostream& c) const {
	  c << m_state << endl;
	}

      private:
	/**
	 * Keep track of the state of the registers. This
	 * is needed to make sure that a change from the cache
	 * propagates into the hardware properly.
	 */
	blitz::Array<State, 1> m_state;
      };
  };
};

#endif /* REGISTERSTATE_H_ */
