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

/*
 * Implements the following sequences:
 *
 *    idle_state  <------------------------------------\
 *    ||                                               |
 *    |\-> RSUpreclear_state                           |   RSUCLEAR_WAIT
 *    |	   clearClock_state  <--------------------\    |
 *    |	   writeClock_state  <----------------\   |    |
 *    |	   readClock_state    --> readError --/   |    |   TDREAD_TIMEOUT
 *  /-|--->RCUdisable_state   --> writeError -----/    |   WRITE_TIMEOUT
 *  | |	   | (ok)             --> ok & finalState -----/
 *  | |    v
 *  | \--> RSUclear_state    <-------------------\         RSUCLEAR_WAIT
 *  |      setBlocksync_state --> writeError --->+         WRITE_TIMEOUT
 *  |      RADwrite_state     --> writeError --->+         WRITE_TIMEOUT
 *  |      PPSsync_state      --> writeError --->+         WRITE_TIMEOUT
 *  |      RCUenable_state    --> writeError --->+         WRITE_TIMEOUT
 *  |      CDOenable_state    --> writeError ----/         WRITE_TIMEOUT
 *  | 	                      --> finalState=True --\
 *  \<----------------------------------------------/
 *
 */

/**
 * Instance pointer for the Cache singleton class.
 */
Sequencer* Sequencer::m_instance = 0;

Sequencer::Sequencer() : 
	GCFFsm       ((State)&Sequencer::idle_state), 
	itsIdle      (true), 
	itsCurSeq    (SEQ_NONE), 
	itsFinalState(false)
{ }

Sequencer& Sequencer::getInstance()
{
	if (!m_instance) {
		m_instance = new Sequencer;
	}
	return *m_instance;
}

Sequencer::~Sequencer()
{}

void Sequencer::run(GCFEvent& event, GCFPortInterface& port)
{
	this->doEvent(event, port);
}

bool Sequencer::isActive() const
{
	return (!itsIdle);
}

GCFEvent::TResult Sequencer::idle_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_INIT:
	break;

	case F_ENTRY: {
		LOG_INFO("Entering Sequencer::idle_state");
		itsIdle = true;
	}
	break;

	case F_TIMER: {
		if (GET_CONFIG("RSPDriver.DISABLE_INIT", i) == 0) {
			if (itsCurSeq == SEQ_SETCLOCK) {
				Cache::getInstance().reset();
				TRAN(Sequencer::RSUpreclear_state);
			} else if (itsCurSeq == SEQ_RSPCLEAR) {
				TRAN(Sequencer::RSUclear_state);
			}
		}
	}
	break;

	case F_EXIT: {
		LOG_DEBUG("Leaving Sequencer::idle_state");
		itsIdle       = false;
		itsFinalState = false;
	}
	break;

	default:
	break;
	}

	return (GCFEvent::HANDLED);
}

//
// RSUpreclear_state(event, port)
//
GCFEvent::TResult Sequencer::RSUpreclear_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Entering Sequencer::RSUpreclear_state");
        
		// Change the register to set the clear flag
		RSUSettings::ResetControl 	rsumode;
		rsumode.setClear(true);
		for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
			Cache::getInstance().getBack().getRSUSettings()()(rsp) = rsumode;
		}

		// signal that the register has changed
		Cache::getInstance().getState().rsuclear().reset();
		Cache::getInstance().getState().rsuclear().write();
		itsTimer = 0;
		break;
	}

	case F_TIMER:
		if (itsTimer++ > RSUCLEAR_WAIT && Cache::getInstance().getState().rsuclear().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::clearClock_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::RSUpreclear_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// clearClock_state(event, port)
//
GCFEvent::TResult Sequencer::clearClock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::clearClock_state");
		Cache::getInstance().getState().tdclear().reset();
		Cache::getInstance().getState().tdclear().write();
		break;

	case F_TIMER:
		if (Cache::getInstance().getState().tdclear().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::writeClock_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::clearClock_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// writeClock_state(event, port)
//
GCFEvent::TResult Sequencer::writeClock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::writeClock_state");
		Cache::getInstance().getState().tdwrite().reset();
		Cache::getInstance().getState().tdwrite().write();
		break;

	case F_TIMER:
		if (Cache::getInstance().getState().tdwrite().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::readClock_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::writeClock_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// readClock_state(event, port)
//
GCFEvent::TResult Sequencer::readClock_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::readClock_state");
		Cache::getInstance().getState().tdread().reset();
		Cache::getInstance().getState().tdread().read();
		itsTimer = 0;
		break;

	case F_TIMER:
		if (Cache::getInstance().getState().tdread().getMatchCount(RegisterState::READ_ERROR) > 0) {
			if (itsTimer++ > TDREAD_TIMEOUT) {
				LOG_WARN("Failed to verify setting of clock. Retrying...");
				TRAN(Sequencer::writeClock_state);
			} else {
				// read again
				Cache::getInstance().getState().tdread().reset();
				Cache::getInstance().getState().tdread().read();
			}
		} else if (Cache::getInstance().getState().tdread().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::RCUdisable_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::readClock_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// RCUdisable_state(event, port)
//
GCFEvent::TResult Sequencer::RCUdisable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::RCUdisable_state");
		enableRCUs(false);
		itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT &&
				Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to disable receivers. Retrying...");
			itsFinalState = false;
			TRAN(Sequencer::clearClock_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
			if (itsFinalState) {
				stopSequence();
				TRAN(Sequencer::idle_state);
			}
			else {
				TRAN(Sequencer::RSUclear_state);
			}
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::RCUdisable_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// RSUclear(event, port)
//
GCFEvent::TResult Sequencer::RSUclear_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Entering Sequencer::RSUclear_state");

		// Change the register to set the clear flag
		RSUSettings::ResetControl rsumode;
		rsumode.setClear(true);
		for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
			Cache::getInstance().getBack().getRSUSettings()()(rsp) = rsumode;
		}

		// signal that the register has changed
		Cache::getInstance().getState().rsuclear().reset();
		Cache::getInstance().getState().rsuclear().write();

		itsTimer = 0;
		break;
	}

	case F_TIMER:
		if (itsTimer++ > RSUCLEAR_WAIT && Cache::getInstance().getState().rsuclear().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::setBlocksync_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::RSUclear_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// setBlocksync_state(event, port)
//
GCFEvent::TResult Sequencer::setBlocksync_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::setBlocksync_state");
		Cache::getInstance().getState().bs().reset();
		Cache::getInstance().getState().bs().write();
		itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT &&
				Cache::getInstance().getState().bs().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to set BS (blocksync) register. Retrying...");
			Cache::getInstance().getState().bs().reset();
			TRAN(Sequencer::RSUclear_state);
		} else if (Cache::getInstance().getState().bs().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::RADwrite_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::setBlocksync_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// RADwrite_state(event, port)
//
GCFEvent::TResult Sequencer::RADwrite_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::RADwrit_state");
		Cache::getInstance().getState().rad().reset();
		Cache::getInstance().getState().rad().write();
		itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT &&
				Cache::getInstance().getState().rad().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to write RAD settings register. Retrying...");
			TRAN(Sequencer::RSUclear_state);
		} else if (Cache::getInstance().getState().rad().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::PPSsync_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::RADwrite_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// PPSsync_state(event, port)
//
GCFEvent::TResult Sequencer::PPSsync_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::PPSsync_state");
		Cache::getInstance().getState().crcontrol().reset(); // set to IDLE
		Cache::getInstance().getState().crcontrol().read();  // set to READ
		// Note: we set the state to read iso write so that the CRSync action knows it a new start.
		//       It will send a 'reset' to the registers first and than change the state to write during
		//		 the repeated writes till all APs have the right delay.
		itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT &&
				Cache::getInstance().getState().crcontrol().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to write PPSsync settings register. Retrying...");
			stringstream	ss;
			Cache::getInstance().getState().crcontrol().print(ss);
			LOG_DEBUG_STR("PPSsync failure state: " << ss);
			TRAN(Sequencer::RSUclear_state);
		} else if (Cache::getInstance().getState().crcontrol().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::RCUenable_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::PPSsync_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
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
// RCUenable_state(event, port)
//
GCFEvent::TResult Sequencer::RCUenable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	static bool	waitForOddSecond(false);

	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Entering Sequencer::RCUenable_state");
		itsTimer = 0;

		// command may only be executed on even seconds for RTCP
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
		if (itsTimer++ > WRITE_TIMEOUT && 
			Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to enable receivers. Retrying...");
			TRAN(Sequencer::RSUclear_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
			TRAN(Sequencer::CDOenable_state);
		}
	}
	break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::RCUenable_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// CDOenable_state(event, port)
//
GCFEvent::TResult Sequencer::CDOenable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::CDOenable_state");
		Cache::getInstance().getState().cdo().reset();
		Cache::getInstance().getState().cdo().write();
        itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT &&
				Cache::getInstance().getState().rcusettings().getMatchCount(RegisterState::WRITE) > 0) {
			LOG_WARN("Failed to enable receivers. Retrying...");
			TRAN(Sequencer::RSUclear_state);
		} else if (Cache::getInstance().getState().rcusettings().isMatchAll(RegisterState::IDLE)) {
            if (StationSettings::instance()->hasAartfaac()) {
                TRAN(Sequencer::SDObitmode_state);
            }
            else {
                itsFinalState = true;
                TRAN(Sequencer::RCUdisable_state);
            }
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::CDOenable_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// CDOenable_state(event, port)
//
GCFEvent::TResult Sequencer::SDObitmode_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	int select;
    int bits_per_sample;
    switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::SDObitmode_state");
        select = 0;
        bits_per_sample = GET_CONFIG("RSPDriver.SDO_MODE", i);
        if      (bits_per_sample == 8) { select = 1; } 
        else if (bits_per_sample == 5) { select = 2; } 
        else if (bits_per_sample == 4) { select = 3; } 
        RSRSDOMode sdomodeinfo;
        sdomodeinfo.bm_select = select;
        sdomodeinfo.bm_max = 3;
        for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
			Cache::getInstance().getBack().getSDOModeInfo()()(rsp) = sdomodeinfo;
			Cache::getInstance().getFront().getSDOModeInfo()()(rsp) = sdomodeinfo;
		}
		Cache::getInstance().getState().sdoState().reset();
		Cache::getInstance().getState().sdoState().write();
        
        itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT && Cache::getInstance().getState().sdoState().isMatchAll(RegisterState::IDLE)) {
            //itsFinalState = true;
			TRAN(Sequencer::SDOselect_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::SDObitmode_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

// default settings
// sdo_ss=295:330,331:366,367:402,403:438
blitz::Array<uint16, 2> Sequencer::str2blitz(const char* str, int max)
{
	string inputstring(str);
	char* start  = (char*)inputstring.c_str();
	char* end  = 0;
	bool  range  = false;
	long  prevval = 0;
    
    blitz::Array<uint16, 2> ss(4,36); // ss = subband select
	int bank_nr = 0;
	int sb_nr = 0;
    long i;
	
    ss = 0;
	while (start) {
		long val = strtol(start, &end, 10); // read decimal numbers
		start = (end ? (*end ? end + 1 : 0) : 0); // advance
		if (val >= max || val < 0) {
			LOG_WARN(formatString("Error: value %ld out of range",val));
			ss = 0;
			return ss;
		}
        LOG_INFO_STR("val=" << val << "  prevval=" << prevval);
		if (end) {
			switch (*end) {
                case ',':
                case 0: {
                    if (range) {
                        if (0 == prevval && 0 == val) {
                            val = max - 1;
                        }
                        if (val < prevval) {
                            LOG_WARN("Error: invalid range specification");
                            ss = 0;
                            return ss;
                        }
                        
                        for (i = prevval; i <= val; i++) {
                            //LOG_INFO(formatString("add value %ld to ss(%d,%d)", i, bank_nr, sb_nr)); 
                            ss(bank_nr, sb_nr) = (uint16)i;
                            sb_nr++;
                            if (sb_nr >= 36) {
                                bank_nr++;
                                sb_nr = 0;
                            }
                        }
					}
					else {
						ss(bank_nr, sb_nr) = (uint16)val;
                        sb_nr++;
                        if (sb_nr >= 36) {
                            bank_nr++;
                            sb_nr = 0;
                        }
					}
					range=false;
				} break;

                case ':': {
                    range=true;
				} break;

                default: {
                    LOG_WARN(formatString("Error: invalid character %c",*end));
                    ss = 0;
                    return ss;
				} break;
			} // switch
		} // if (end)
        prevval = val;
	} // while
        
	return (ss);
}

//
// CDOenable_state(event, port)
//
GCFEvent::TResult Sequencer::SDOselect_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	char select_str[64];
    int nBanks;
    int pol;
    blitz::Array<uint16, 2> select(4,36);
    
    switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::SDOselect_state");
        strncpy(select_str, GET_CONFIG_STRING("RSPDriver.SDO_SS"), 64);
		LOG_DEBUG_STR("select string = " << select_str);
        select = str2blitz(select_str, 512);
        LOG_DEBUG_STR("SDO select values = " << select);
        nBanks = (MAX_BITS_PER_SAMPLE / MIN_BITS_PER_SAMPLE); // fill all banks
        for (int rcu = 0; rcu < StationSettings::instance()->nrRcus(); rcu++) {
            pol = rcu%2;
            for (int bank = 0; bank < nBanks; bank++) {
                Cache::getInstance().getBack().getSDOSelection().subbands()(rcu, bank, Range::all()) = 0;
                for (int sb = 0; sb < 36; sb++) {
                    Cache::getInstance().getBack().getSDOSelection().subbands()(rcu, bank, sb) = (select(bank, sb) * 2) + pol;
                    Cache::getInstance().getFront().getSDOSelection().subbands()(rcu, bank, sb) = (select(bank, sb) * 2) + pol;
                } // for each subband
            } // for each bank

            if (rcu == 0) {
                LOG_DEBUG_STR("cache->subbands.sdo ss(0) = " << Cache::getInstance().getBack().getSDOSelection().subbands()(0, Range::all(), Range::all()));
            }
        } // for each rcu
        
        Cache::getInstance().getState().sdoSelectState().reset();
		Cache::getInstance().getState().sdoSelectState().write();
		
        itsTimer = 0;
        break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT && Cache::getInstance().getState().sdoSelectState().isMatchAll(RegisterState::IDLE)) {
            //itsFinalState = true;
			TRAN(Sequencer::SDOenable_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::SDOselect_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// CDOenable_state(event, port)
//
GCFEvent::TResult Sequencer::SDOenable_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
    switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Entering Sequencer::SDOenable_state");
        for (int blp_nr = 0; blp_nr < StationSettings::instance()->nrBlps(); blp_nr += 4) {
            Cache::getInstance().getBack().getBypassSettings()()(blp_nr).setSDO(1);
            Cache::getInstance().getFront().getBypassSettings()()(blp_nr).setSDO(1);
            Cache::getInstance().getState().bypasssettings().reset(blp_nr);
            Cache::getInstance().getState().bypasssettings().write(blp_nr);
        }
        itsTimer = 0;
		break;

	case F_TIMER:
		if (itsTimer++ > WRITE_TIMEOUT && Cache::getInstance().getState().bypasssettings().isMatchAll(RegisterState::IDLE)) {
			itsFinalState = true;
			TRAN(Sequencer::RCUdisable_state);
		}
		break;

	case F_EXIT:
		LOG_DEBUG("Leaving Sequencer::SDOenable_state");
		break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}



  } // namespace RSP
} // namespace LOFAR

