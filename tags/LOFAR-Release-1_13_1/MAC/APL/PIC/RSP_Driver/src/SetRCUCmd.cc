//#  SetRCUCmd.cc: implementation of the SetRCUCmd class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "CableSettings.h"
#include "SetRCUCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetRCUCmd::SetRCUCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetRCU", port, oper)
{
	m_event = new RSPSetrcuEvent(event);

	LOG_INFO(formatString("control=0x%08x", m_event->settings()(0).getRaw()));
}

SetRCUCmd::~SetRCUCmd()
{
	delete m_event;
}

void SetRCUCmd::ack(CacheBuffer& /*cache*/)
{
	RSPSetrcuackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;
	getPort()->send(ack);
}

void SetRCUCmd::apply(CacheBuffer& cache, bool setModFlag)
{
	// someone else using the I2C bus?
	I2Cuser	busUser = cache.getI2Cuser();
	LOG_INFO_STR("SetRCU::apply : " << ((busUser == NONE) ?  "NONE" : 
									   ((busUser == HBA) ?   "HBA" : 
									   ((busUser == RCU_R) ? "RCU_R" : "RCU_W"))));
	if (busUser != NONE && busUser != RCU_W) {
		postponeExecution(true);
		return;
	}
	cache.setI2Cuser(RCU_W);		// claim the I2C bus.
	postponeExecution(false);

	bool			newMode 	  = m_event->settings()(0).isModeModified();
	uint32			mode 		  = m_event->settings()(0).getMode();
	CableSettings*	cableSettings = CableSettings::instance();
	float			delayStep	  = 1000.0 / cache.getClock();

//  LOG_INFO("SetRCUCmd::apply");
	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
		if (m_event->rcumask[cache_rcu]) {
			// make change
			cache.getRCUSettings()()(cache_rcu) = m_event->settings()(0);
			
			// Apply delays and attenuation when mode was changed.
			if (newMode) {
				cache.getRCUSettings()()(cache_rcu).setDelay(
							(uint8) ((delayStep/2.0 + cableSettings->getDelay(cache_rcu, mode)) / delayStep));
				cache.getRCUSettings()()(cache_rcu).setAttenuation(
							(uint8) ((-0.125 + cableSettings->getAtt(cache_rcu, mode)) / -0.25));
				if (cache_rcu == 0) {
					LOG_DEBUG(formatString("RCU 0 new Delay   : %f/2.0 + %f / %f = %d", 
						delayStep, cableSettings->getDelay(0, mode), delayStep, 
						(uint8) ((delayStep/2.0 + cableSettings->getDelay(cache_rcu, mode)) / delayStep)));
					LOG_DEBUG(formatString("RCU 0 new Atten   : -0.125  + %f / -0.25 = %d", cableSettings->getAtt(0, mode),
						(uint8) ((-0.125 + cableSettings->getAtt(cache_rcu, mode)) / -0.25)));
					LOG_DEBUG(formatString("RCU 0 new RawMode : %08lX", cache.getRCUSettings()()(0).getRaw()));
				}
			}

			if (setModFlag) {
				// only write RCU Handler settings if modified
				if (m_event->settings()(0).isHandlerModified()) {
					cache.getCache().getState().rcusettings().write(cache_rcu);
				}

				// only write RCU Protocol settings if modified
				if (m_event->settings()(0).isProtocolModified()) {
					cache.getCache().getState().rcuprotocol().write(cache_rcu);
				}
			}
		} // if in mask
	} // for
}

void SetRCUCmd::complete(CacheBuffer& cache)
{
	ack(cache);
}

const Timestamp& SetRCUCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetRCUCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetRCUCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
	  && (1 == m_event->settings().dimensions())
	  && (1 == m_event->settings().extent(firstDim)));
}
