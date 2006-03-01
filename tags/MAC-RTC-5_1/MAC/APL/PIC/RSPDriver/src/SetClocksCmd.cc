//#  SetClocksCmd.cc: implementation of the SetClocksCmd class
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
#include "SetClocksCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetClocksCmd::SetClocksCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSetclocksEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

SetClocksCmd::~SetClocksCmd()
{
  delete m_event;
}

void SetClocksCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetclocksackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status    = SUCCESS;
  
  getPort()->send(ack);
}

void SetClocksCmd::apply(CacheBuffer& cache, bool setModFlag)
{
  for (int cache_rsp = 0; cache_rsp < StationSettings::instance()->nrRspBoards(); cache_rsp++) {
    if (m_event->rspmask[cache_rsp]) {
      cache.getClocks()()(cache_rsp) = m_event->clocks()(0);
      if (setModFlag) {
        cache.getClocks().getState().modified(cache_rsp);
	  }
    }
  }
}

void SetClocksCmd::complete(CacheBuffer& /*cache*/)
{
  LOG_INFO_STR("SetClocksCmd completed at time=" << getTimestamp());
}

const Timestamp& SetClocksCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetClocksCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetClocksCmd::validate() const
{
  bool values_ok = true;
  for (int i = 0; i < m_event->clocks().extent(firstDim); i++) {
    if (  0 != m_event->clocks()(i) &&
	160 != m_event->clocks()(i) &&
	200 != m_event->clocks()(i)) {
      values_ok = false;
      break;
    }
  }
  return (values_ok
	  && (m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards())
	  && (1 == m_event->clocks().dimensions())
	  && (1 == m_event->clocks().extent(firstDim)));
}
