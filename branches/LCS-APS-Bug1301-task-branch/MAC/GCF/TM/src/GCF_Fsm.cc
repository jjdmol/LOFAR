//#  GCFFsm.cc: implementation of Finite State Machine.
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <GCF/TM/GCF_Fsm.h>
#include <GTM_Defines.h>

namespace LOFAR {
 using MACIO::GCFEvent;
 namespace GCF {
  namespace TM {

// static data member initialisation
GCFDummyPort GCFFsm::_gcfPort(0, "GCFFSM", F_FSM_PROTOCOL);

//
// initFsm()
//
void GCFFsm::initFsm()
{
	GCFEvent e;
	e.signal = F_ENTRY;
	(void)(this->*_state)(e, _gcfPort); // entry signal
	e.signal = F_INIT;
	if (GCFEvent::HANDLED != (this->*_state)(e, _gcfPort)) { // initial transition
		LOG_FATAL(LOFAR::formatString (
			"Fsm::init: initial transition F_SIGNAL(F_FSM_PROTOCOL, F_INIT) not handled."));
		exit(1);
	}
}

//
// tran(target, from, to)
//
void GCFFsm::tran(State target, const char* from, const char* to)
{
	GCFEvent e;
	e.signal = F_EXIT;
	(void)(this->*_state)(e, _gcfPort); // exit signal

	LOG_DEBUG(LOFAR::formatString ( "State transition to %s <<== %s", to, from));

	_state = target; // state transition

	e.signal = F_ENTRY;
	(void)(this->*_state)(e, _gcfPort); // entry signal
}

//
// quitFsm()
//
void GCFFsm::quitFsm()
{
	GCFEvent	event;
	event.signal = F_QUIT;
	(void)(this->*_state)(event, _gcfPort);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
