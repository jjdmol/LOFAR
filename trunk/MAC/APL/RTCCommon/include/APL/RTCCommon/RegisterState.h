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
#include <APL/RTCCommon/Marshalling.h>
#include <iostream>

namespace LOFAR {
  namespace RTC {

    class RegisterState
      {
      public:

	typedef enum State {
	  UNDEFINED = 0,
	  IDLE,
	  CHECK,
	  WRITE,
	  READ,
	  WRITE_ERROR,
	  READ_ERROR,
	  DONE,
	};
	
	explicit RegisterState(State state = UNDEFINED)
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
	 *                                     read_error
	 *                                         +---> READ_ERROR
	 *            reset          read          |
	 *               \     +--------------->  READ ------+ read_ack
	 *                \    |                    *        |
	 *                 *   |           schedule_|read    *       clear
	 * UNDEFINED ---> IDLE + <---+              |       DONE --------------->+
	 *                 ^   | not_|modified      |        ^                   |
	 *                 |   |     |              |        |                   |
	 *                 |   +--> CHECK------> WRITE ------+                   |
	 *                 |  check       write    |      write_ack              |
	 *                 |                       +---> WRITE_ERROR             |
	 *                 |          write_error                                |
	 *                 +---------------<-------------------------------------+
	 *
	 *
	 * Rationale:
	 * At each update cycle all registers should be in the IDLE state.
	 * All registers are then moved to the READ or CHEC state depending
	 * on whether they are R (read) or W (write) registers.
	 *
	 * States:
	 * UNDEFINED, IDLE, CHECK, WRITE, READ, WRITE_ERROR, READ_ERROR, DONE
	 *
	 * Signals:
	 * read, check, unmodified, write, schedule_read, read_ack, write_ack, read_error, write_error, clear, reset
	 */

	void resize(int n)
	  {
	    m_state.resize(n);
	    m_state = IDLE;
	  }

	void tran(State source, State target, int i);

	void read         (int i = -1) { tran(IDLE,          READ,          i); }
	void check        (int i = -1) { tran(IDLE,          CHECK,         i); }
	void write_force  (int i = -1) { tran(IDLE,          WRITE,         i); }
	void unmodified   (int i = -1) { tran(CHECK,         IDLE,          i); }
	void write        (int i = -1) { tran(CHECK,         WRITE,         i); }
	void read_schedule(int i = -1) { tran(WRITE,         READ,          i); }
	void read_ack     (int i = -1) { tran(READ,          DONE,          i); }
	void write_ack    (int i = -1) { tran(WRITE,         DONE,          i); }
	void read_error   (int i = -1) { tran(READ,          READ_ERROR,    i); }
	void write_error  (int i = -1) { tran(WRITE,         WRITE_ERROR,   i); }

	void clear(int i = -1);
	void reset(int i = -1);

	State get(int i) {
	  ASSERT(i >= 0 && i < m_state.extent(blitz::firstDim));
	  return m_state(i);
	}

	void print(std::ostream& out) const {
	  for (int i = 0; i < m_state.extent(blitz::firstDim); i++) {
	    switch (m_state(i)) {
	    case UNDEFINED:     out << "? "; break;
	    case IDLE:          out << ". "; break;
	    case CHECK:         out << "C "; break;
	    case WRITE:         out << "W "; break;
	    case READ:          out << "R "; break;
	    case READ_ERROR:    out << "ER"; break;
	    case WRITE_ERROR:   out << "EW"; break;
	    case DONE:          out << "* "; break;
	    default:            out << "X "; break;
	    }
	  }
	  out << "$" << endl;
	}

      public:
	/* marshalling methods */
	unsigned int getSize() {
	  return MSH_ARRAY_SIZE(m_state, State);
	}

	unsigned int pack(void* buffer) {
	  unsigned int offset = 0;

	  MSH_PACK_ARRAY(buffer, offset, m_state, State);

	  return offset;
	}

	unsigned int unpack(void* buffer) {
	  unsigned int offset = 0;

	  MSH_UNPACK_ARRAY(buffer, offset, m_state, State, 1);

	  return offset;
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
