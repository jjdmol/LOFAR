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

#include <RSP_Protocol/RSP_Protocol.ph>
#include <PSAccess.h>
#include <blitz/array.h>

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
  ack.status = SUCCESS;
  
  getPort()->send(ack);
}

void SetClocksCmd::apply(CacheBuffer& cache)
{
  for (int cache_td = 0; cache_td < GET_CONFIG("RS.N_TDBOARDS", i); cache_td++)
  {
    if (m_event->tdmask[cache_td])
    {
      cache.getClocks()()(cache_td) = m_event->clocks()(0);
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
  return ((m_event->tdmask.count() <= (unsigned int)GET_CONFIG("RS.N_TDBOARDS", i))
	  && (1 == m_event->clocks().dimensions())
	  && (1 == m_event->clocks().extent(firstDim)));
}
