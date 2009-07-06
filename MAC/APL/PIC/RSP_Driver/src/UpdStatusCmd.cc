//#  UpdStatusCmd.cc: implementation of the UpdStatusCmd class
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
#include "UpdStatusCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

UpdStatusCmd::UpdStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SubStatus", port, oper)
{
  m_event = new RSPSubstatusEvent(event);

  setPeriod(m_event->period);
}

UpdStatusCmd::~UpdStatusCmd()
{
  delete m_event;
}

void UpdStatusCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdStatusCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void UpdStatusCmd::complete(CacheBuffer& cache)
{
  RSPUpdstatusEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;
  ack.handle = (memptr_t)this; // opaque pointer used to refer to the subscription

  ack.sysstatus.board().resize(m_event->rspmask.count());

  int result_rsp = 0;
  for (int cache_rsp = 0; cache_rsp < StationSettings::instance()->nrRspBoards(); cache_rsp++)
  {
    if (m_event->rspmask[cache_rsp])
    {
      ack.sysstatus.board()(result_rsp) = cache.getSystemStatus().board()(cache_rsp);
      result_rsp++;
    }
  }

  getPort()->send(ack);
}

const Timestamp& UpdStatusCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdStatusCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdStatusCmd::validate() const
{
  return (m_event->rspmask.count() <= (unsigned int)StationSettings::instance()->nrRspBoards());
}
