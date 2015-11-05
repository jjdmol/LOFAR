//#  SetTBBCmd.cc: implementation of the SetTBBCmd class
//#
//#  Copyright (C) 2002-2004
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
#include "SetTBBCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetTBBCmd::SetTBBCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetTBB", port, oper)
{
  m_event = new RSPSettbbEvent(event);
}

SetTBBCmd::~SetTBBCmd()
{
  delete m_event;
}

void SetTBBCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSettbbackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetTBBCmd::apply(CacheBuffer& cache, bool setModFlag)
{
  for (int cache_rcu = 0;
       cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
    if (m_event->rcumask[cache_rcu]) {
      cache.getTBBSettings()()(cache_rcu) = m_event->settings()(0);
      if (setModFlag) {
	cache.getCache().getState().tbbsettings().write(cache_rcu);
        cache.getCache().getState().tbbbandsel().write(cache_rcu);
      }
    }
  }
}

void SetTBBCmd::complete(CacheBuffer& /*cache*/)
{
//  LOG_INFO_STR("SetTBBCmd completed at time=" << getTimestamp());
}

const Timestamp& SetTBBCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetTBBCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetTBBCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
	  && (1 == m_event->settings().dimensions())
	  && (1 == m_event->settings().extent(firstDim)));
}
