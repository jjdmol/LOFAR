//#  Sequencer.cc: implementation of the Sequencer class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "Sequencer.h"
#include "Cache.h"
#include "StationSettings.h"

using namespace blitz;
namespace LOFAR {
	using namespace GCF::TM;
	using namespace EPA_Protocol;
	using namespace RTC;
	namespace RSP {

#define TDREAD_TIMEOUT 3
#define RSUCLEAR_WAIT  5
#define WRITE_TIMEOUT  3

/**
 * Instance pointer for the Cache singleton class.
 */
Sequencer* Sequencer::m_instance = 0;

Sequencer::Sequencer()
  : GCFFsm((State)&Sequencer::idle_state), m_active(false), m_sequence(NONE), itsFinalState(false)
{
}

Sequencer& Sequencer::getInstance()
{
	if (0 == m_instance) {
		m_instance = new Sequencer;
	}

	return *m_instance;
}

Sequencer::~Sequencer()
{
}

void Sequencer::run(GCFEvent& event, GCFPortInterface& port)
{
	this->doEvent(event, port);
}

bool Sequencer::isActive() const
{
	return m_active;
}

GCFEvent::TResult Sequencer::idle_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: {
	}
	break;

	case F_ENTRY: {
		LOG_DEBUG("Entering Sequencer::idle_state");
		m_active = false;
	}
	break;

	case F_TIMER: {
		if (!GET_CONFIG("RSPDriver.DISABLE_INIT", i)) {
			if (SETCLOCK == m_sequence) {
				Cache::getInstance().reset();
				TRAN(Sequencer::rsupreclear_state);
			} else if (RSPCLEAR == m_sequence) {
				TRAN(Sequencer::rsuclear_state);
			}
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::idle_state");
		m_active = true;
		itsFinalState = false;
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return GCFEvent::HANDLED;
}

//
// rsupreclear_state(event, port)
//
GCFEvent::TResult Sequencer::rsupreclear_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::rsuclear_state");

      // Change the regsiter to set the clear flag
      RSUSettings::ResetControl rsumode;
      rsumode.setClear(true);
      for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
	Cache::getInstance().getBack().getRSUSettings()()(rsp) = rsumode;
      }

      // signal that the register has changed
      Cache::getInstance().getState().rsuclear().reset();
      Cache::getInstance().getState().rsuclear().write();

      m_timer = 0;
    }
    break;
    
    case F_TIMER: {
      if (m_timer++ > RSUCLEAR_WAIT && Cache::getInstance().getState().rsuclear().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::clearclock_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::rsuclear_state");
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// clearclock_state(event, port)
//
GCFEvent::TResult Sequencer::clearclock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::clearclock_state");
      Cache::getInstance().getState().tdclear().reset();
      Cache::getInstance().getState().tdclear().write();

    }
    break;

    case F_TIMER: {
      if (Cache::getInstance().getState().tdclear().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::writeclock_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::clearclock_state");
    }
    break;
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// writeclock_state(event, port)
//
GCFEvent::TResult Sequencer::writeclock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::writeclock_state");
      Cache::getInstance().getState().tdwrite().reset();
      Cache::getInstance().getState().tdwrite().write();
    }
    break;
    
    case F_TIMER: {
      if (Cache::getInstance().getState().tdwrite().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::readclock_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::writeclock_state");
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// readclock_state(event, port)
//
GCFEvent::TResult Sequencer::readclock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::readclock_state");
      Cache::getInstance().getState().tdread().reset();
      Cache::getInstance().getState().tdread().read();
      
      m_timer = 0;
    }
    break;
    
    case F_TIMER: {
      if (Cache::getInstance().getState().tdread().getMatchCount(RegisterState::READ_ERROR) > 0) {
	if (m_timer++ > TDREAD_TIMEOUT) {
	  LOG_WARN("Failed to verify setting of clock. Retrying...");
	  TRAN(Sequencer::writeclock_state);
	} else {
	  // read again
	  Cache::getInstance().getState().tdread().reset();
	  Cache::getInstance().getState().tdread().read();
	}
      } else if (Cache::getInstance().getState().tdread().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::rcudisable_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::readclock_state");
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// rcudisable_state(event, port)
//
GCFEvent::TResult Sequencer::rcudisable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Entering Sequencer::rcudisable_state");
		enableRCUs(false);
		m_timer = 0;
	}
	break;

	case F_TIMER: {
		if (m_timer++ > WRITE_TIMEOUT
			&& Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to disable receivers. Retrying...");
			itsFinalState = false;
			TRAN(Sequencer::clearclock_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
			if (itsFinalState) {
				stopSequence();
				TRAN(Sequencer::idle_state);
			}
			else {
				TRAN(Sequencer::rsuclear_state);
			}
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::rcudisable_state");
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return GCFEvent::HANDLED;
}

//
// writeclock_state(event, port)
//
GCFEvent::TResult Sequencer::rsuclear_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::rsuclear_state");

      // Change the regsiter to set the clear flag
      RSUSettings::ResetControl rsumode;
      rsumode.setClear(true);
      for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
	Cache::getInstance().getBack().getRSUSettings()()(rsp) = rsumode;
      }

      // signal that the register has changed
      Cache::getInstance().getState().rsuclear().reset();
      Cache::getInstance().getState().rsuclear().write();

      m_timer = 0;
    }
    break;
    
    case F_TIMER: {
      if (m_timer++ > RSUCLEAR_WAIT && Cache::getInstance().getState().rsuclear().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::setblocksync_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::rsuclear_state");
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// setblocksync_state(event, port)
//
GCFEvent::TResult Sequencer::setblocksync_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal) {
    case F_ENTRY: {
      LOG_DEBUG("Entering Sequencer::setblocksync_state");
      Cache::getInstance().getState().bs().reset();
      Cache::getInstance().getState().bs().write();
      m_timer = 0;
    }
    break;
    
    case F_TIMER: {
      if (   m_timer++ > WRITE_TIMEOUT
	  && Cache::getInstance().getState().bs().getMatchCount(RegisterState::WRITE) > 0) {
	LOG_WARN("Failed to set BS (blocksync) register. Retrying...");
	Cache::getInstance().getState().bs().reset();
	TRAN(Sequencer::rsuclear_state);
      } else if (Cache::getInstance().getState().bs().isMatchAll(RegisterState::IDLE)) {
	TRAN(Sequencer::radwrite_state);
      }
    }
    break;

    case F_EXIT: {
      LOG_DEBUG("Leaving Sequencer::setblocksync_state");
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return GCFEvent::HANDLED;
}

//
// radwrite_state(event, port)
//
GCFEvent::TResult Sequencer::radwrite_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Entering Sequencer::radwrit_state");
		Cache::getInstance().getState().rad().reset();
		Cache::getInstance().getState().rad().write();

		m_timer = 0;
	}
	break;

	case F_TIMER: {
		if (m_timer++ > WRITE_TIMEOUT 
			&& Cache::getInstance().getState().rad().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to write RAD settings register. Retrying...");
			TRAN(Sequencer::rsuclear_state);
		} else if (Cache::getInstance().getState().rad().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::rcuenable_state);
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::radwrite_state");
		}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return GCFEvent::HANDLED;
}

//
// enableRCUs [private]
//
void Sequencer::enableRCUs(bool	on)
{
	RCUSettings::Control control;
	control.setEnable(on ? 1 : 0);

	Cache::getInstance().getFront().getRCUSettings()() = control;
	Cache::getInstance().getBack().getRCUSettings()() = control;

	Cache::getInstance().getState().rcusettings().reset();
	Cache::getInstance().getState().rcusettings().write();
}

//
// rcuenable_state(event, port)
//
GCFEvent::TResult Sequencer::rcuenable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	static bool	waitForOddSecond(false);

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Entering Sequencer::rcuenable_state");
		m_timer = 0;

		// command may only be executed on even seconds for OLAP
		// since the timestamp is always one second ahead we have
		// to wait of an odd second (to end in the even second).
		if (time(0) % 2 == 0) {
			waitForOddSecond = true;
			LOG_INFO("Wait for even second before enabling RCUs");
			break;
		}

		waitForOddSecond = false;
		LOG_INFO("Entry at even second, enabling RCUs immediately");
		enableRCUs(true);
	}
	break;

	case F_TIMER: {
		if (waitForOddSecond) {
			if (time(0) % 2 == 0) {
				LOG_INFO("Still waiting for even second, missed pps?");
				break;
			}
			waitForOddSecond = false;
			LOG_INFO("Enabling RCUs delayed till even second");
			enableRCUs(true);
			break;
		}

		// Command are sent, wait for command to complete.
		if (m_timer++ > WRITE_TIMEOUT && 
			Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to enable receivers. Retrying...");
			TRAN(Sequencer::rsuclear_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::cdoenable_state);
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::rcuenable_state");
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return GCFEvent::HANDLED;
}

//
// cdoenable_state(event, port)
//
GCFEvent::TResult Sequencer::cdoenable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Entering Sequencer::cdoenable_state");

		Cache::getInstance().getState().cdo().reset();
		Cache::getInstance().getState().cdo().write();
	}
	break;

	case F_TIMER: {
		if (m_timer++ > WRITE_TIMEOUT
			&& Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to enable receivers. Retrying...");
			TRAN(Sequencer::rsuclear_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
			itsFinalState = true;
			TRAN(Sequencer::rcudisable_state);
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::cdoenable_state");
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return GCFEvent::HANDLED;
}

  } // namespace RSP
} // namespace LOFAR

