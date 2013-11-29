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
  using GCF::TM::GCFFsm;
  using GCF::TM::GCFPortInterface;
  using GCF::TM::GCFTimerPort;
  namespace RSP {

class Sequencer : public GCFFsm
{
public:
	typedef enum {
	  SEQ_NONE = 0,
	  SEQ_SETCLOCK, // done at initialization
	  SEQ_RSPCLEAR,
	} Sequence;
	//
	// Constructor/destructor
	//
	static Sequencer& getInstance();
	virtual ~Sequencer();

	//
	// Advance the sequencer state machine
	//
	void run(GCFEvent& event, GCFPortInterface& port);

	//
	// Returns true when the statemachine is not in the 'idle' state.
	//
	bool isActive() const;

	void startSequence(Sequence sequence) { itsCurSeq = sequence; }
	void stopSequence()                   { itsCurSeq = SEQ_NONE; }

	/*@{*/
	//
	// The states of the statemachine.
	//
	GCFEvent::TResult idle_state        (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult RSUpreclear_state (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult clearClock_state  (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult writeClock_state  (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult readClock_state   (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult RCUdisable_state  (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult RSUclear_state    (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult setBlocksync_state(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult RADwrite_state    (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult PPSsync_state     (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult RCUenable_state   (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult CDOenable_state   (GCFEvent& event, GCFPortInterface& port);
	/*@}*/

private:
	//
	// Default construction prohibited (singleton pattern).
	//
	Sequencer();
	void enableRCUs(bool);

	static Sequencer* m_instance;

	bool		itsIdle;		// In idle-state or not
	Sequence	itsCurSeq;		// currently executing sequence

	int		 	itsTimer;		// timer used to delay some actions
	bool		itsFinalState;	// final state of sequence (used by rcudisable_state)
};

  };
};
     
#endif /* SEQUENCER_H_ */
