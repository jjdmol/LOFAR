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

#include <lofar_config.h>
#include <blitz/array.h>
#include <Common/LofarLogger.h>
#include <APL/RTCCommon/MarshallBlitz.h>
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
		FAIL,
		WAIT_1,
		WAIT_2
	};
	static const int MAX_REGISTER_ERROR = 3;

	// constructors /destructor
	explicit RegisterState(State state = UNDEFINED) {
		m_state.resize(1);
		m_state = state;
		m_error.resize(1);
		m_error = 0;
	}
	virtual ~RegisterState() {
		m_state.free();
		m_error.free();
	}


	/**                                
	* RegisterState state machine.
	*                                     read_error
	*                                         +---> READ_ERROR
	*            reset          read          |
	*               \     +--------------->  READ ------+ read_ack
	*                \    |                    *        |
	*                 .   |           schedule_|read    *       clear
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

	void resize(int n) {
		m_state.resize(n);
		m_state = IDLE;
		m_error.resize(n);
		m_error = 0;
	}

	void tran(State source, State target, int i);

	void read       	   (int i = -1) { tran(IDLE,  READ,		   i); }
	void check      	   (int i = -1) { tran(IDLE,  CHECK,	   i); }
	void unmodified 	   (int i = -1) { tran(CHECK, IDLE,		   i); }
	void schedule_read	   (int i = -1) { tran(WRITE, READ,		   i); clearError(i);}
	void schedule_wait1read(int i = -1) { tran(WRITE, WAIT_1,	   i); clearError(i);}
	void schedule_wait2read(int i = -1) { tran(WRITE, WAIT_2,	   i); clearError(i);}
	void read_ack   	   (int i = -1) { tran(READ,  DONE,		   i); clearError(i);}
	void write_ack  	   (int i = -1) { tran(WRITE, DONE,		   i); clearError(i);}
	void read_error 	   (int i = -1) { tran(READ,  READ_ERROR,  i); addError(i); }
	void write_error	   (int i = -1) { tran(WRITE, WRITE_ERROR, i); addError(i); }
	void write      	   (int i = -1);

	void clear(int i = -1);
	void reset(int i = -1);

	State get(int i) const;
	int   getMatchCount(State matchstate) const;
	bool  isMatchAll(State matchstate) const;

	void print(std::ostream& out) const;

	// assignment
	RegisterState& operator=(const RegisterState& state);

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
	// prevent copy
	RegisterState(const RegisterState& state);
	
	// error count functions
	void clearError(int	i) { m_error(i) = 0; }
	void addError  (int	i) { m_error(i)++; }

	/**
	* Keep track of the state of the registers. This
	* is needed to make sure that a change from the cache
	* propagates into the hardware properly.
	*/
	blitz::Array<State, 1> m_state;
	blitz::Array<int, 1>   m_error;
};

  }; // namespace RTC
}; // namespace LOFAR

#endif /* REGISTERSTATE_H_ */
