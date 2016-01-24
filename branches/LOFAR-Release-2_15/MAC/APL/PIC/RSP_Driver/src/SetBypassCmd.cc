//#  SetBypassCmd.cc: implementation of the SetBypassCmd class
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetBypassCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetBypassCmd::SetBypassCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetBypass", port, oper)
{
	m_event = new RSPSetbypassEvent(event);
}

SetBypassCmd::~SetBypassCmd()
{
	delete m_event;
}

void SetBypassCmd::ack(CacheBuffer& /*cache*/)
{
	RSPSetbypassackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;

	getPort()->send(ack);
}

// Note: the SI registers are per BPL iso per RCU. So setting up the right
// bit in the SI register is not straight forward.
void SetBypassCmd::apply(CacheBuffer& cache, bool setModFlag)
{
	bool setSIon  = m_event->settings()(0).getXSI();	// note: X and Y are equal
	bool setSDOon = m_event->settings()(0).getSDO();
    bool siSet    = m_event->settings()(0).isSIset();
    bool sdoSet   = m_event->settings()(0).isSDOset();
	int  board_nr;
    for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
		if (m_event->rcumask[cache_rcu]) {	// is this RCU in the mask?
			// make change
            if (siSet) {
                if (cache_rcu % N_POL == 0) {
                    cache.getBypassSettings()()(cache_rcu/N_POL).setXSI(setSIon);
                }
                else {
                    cache.getBypassSettings()()(cache_rcu/N_POL).setYSI(setSIon);
                }
                // mark register that it should be written.
                if (setModFlag) {
                    cache.getCache().getState().bypasssettings().write(cache_rcu/N_POL);
                }
            }
            if (sdoSet) {
                if ((cache_rcu % 8) == 0) {
                    board_nr = cache_rcu / 8;
                    cache.getBypassSettingsBP()()(board_nr).setSDO(setSDOon);
                    if (setModFlag) {
                        cache.getCache().getState().bypasssettings_bp().write(board_nr);
                    }
                }
            }
		}
	}
}

void SetBypassCmd::complete(CacheBuffer& /*cache*/)
{
//	LOG_DEBUG_STR("SetBypassCmd completed at time=" << getTimestamp());
}

const Timestamp& SetBypassCmd::getTimestamp() const
{
	return m_event->timestamp;
}

void SetBypassCmd::setTimestamp(const Timestamp& timestamp)
{
	m_event->timestamp = timestamp;
}

bool SetBypassCmd::validate() const
{
	
    if ((m_event->settings()(0).isSDOset() == true) && (StationSettings::instance()->hasAartfaac() == false)) {
        LOG_DEBUG("No AARTFAAC available");
        return (false);
    }    
    return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
			&& (1 == m_event->settings().dimensions())
			&& (1 == m_event->settings().extent(firstDim)));
}
