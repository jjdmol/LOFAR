//#  -*- mode: c++ -*-
//#
//#  Sequencer.h: RSP Driver sequencer for state based control of RSP boards
//#               E.g. initialization, change clock, clear, reset
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

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace RSP {

    class Sequencer : public GCFFsm
      {
      public:

	typedef enum {
	  NONE = 0,
	  SETCLOCK, // done at initialization
	  RSPCLEAR,
	} Sequence;
	/*
	 * Constructor/destructor
	 */
	static Sequencer& getInstance();
	virtual ~Sequencer();

	/**
	 * Advance the sequencer state machine
	 */
	void run(GCFEvent& event, GCFPortInterface& port);

	/**
	 * Returns true when the statemachine is not in the 'idle' state.
	 */
	bool isActive() const;

	void startSequence(Sequence sequence) { m_sequence = sequence; }
	void stopSequence()                   { m_sequence = NONE; }

	/*@{*/
	/**
	 * The states of the statemachine.
	 */
	GCFEvent::TResult idle_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult rsupreclear_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult clearclock_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult writeclock_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult readclock_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult rcudisable_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult rsuclear_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult setblocksync_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult radwrite_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult rcuenable_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult cdoenable_state(GCFEvent& event, GCFPortInterface& port);
	/*@}*/

      private:
	/**
	 * Default construction prohibited (singleton pattern).
	 */
	Sequencer();
	void enableRCUs();

	static Sequencer* m_instance;

	bool     m_active;   /* m_active == (state != idle) */
	Sequence m_sequence; /* currently executing sequence */

	int m_timer; /* timer used to delay some actions */
      };
  };
};
     
#endif /* SEQUENCER_H_ */
