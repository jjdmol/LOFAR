//#  SetHBACmd.cc: implementation of the SetHBACmd class
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
#include "SetHBACmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetHBACmd::SetHBACmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetHBA", port, oper)
{
  m_event = new RSPSethbaEvent(event);
}

SetHBACmd::~SetHBACmd()
{
  delete m_event;
}

void SetHBACmd::ack(CacheBuffer& /*cache*/)
{
  RSPSethbaackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetHBACmd::apply(CacheBuffer& cache, bool setModFlag)
{
	int event_rcu = 0; // rcu number in m_event->settings()
	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
		if (m_event->rcumask.test(cache_rcu)) { // check if rcu is selected
      cache.getHBASettings()()(cache_rcu, Range::all()) = m_event->settings()(event_rcu, Range::all());
      if (setModFlag) {
        cache.getCache().getState().hbaprotocol().write(cache_rcu);
				// cache.getCache().getState().hbaprotocol().write(cache_rcu); // waarom deze 2e keer ??
      }
			event_rcu++;
    }
  }
}

void SetHBACmd::complete(CacheBuffer& /*cache*/)
{
  LOG_INFO_STR("SetHBACmd completed at time=" << getTimestamp());
}

const Timestamp& SetHBACmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetHBACmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetHBACmd::validate() const
{
  LOG_INFO_STR(
  	"validate-> rcus=" << m_event->rcumask.count() 
  	<< " dims=" << m_event->settings().dimensions() 
  	<< " ext_firt=" << m_event->settings().extent(firstDim)
  	<< " hba_delays=" << m_event->settings().extent(secondDim) );
  
  return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
	  	&& (2 == m_event->settings().dimensions())
		&& (m_event->rcumask.count() == (unsigned int)m_event->settings().extent(firstDim))		// check number off selected rcus
		&& (MEPHeader::N_HBA_DELAYS == (unsigned int)m_event->settings().extent(secondDim))); // check number of delays
}
