//#  UpdClocksCmd.cc: implementation of the UpdClocksCmd class
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

#include "UpdClocksCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

UpdClocksCmd::UpdClocksCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSubclocksEvent(event);

  setOperation(oper);
  setPeriod(m_event->period);
  setPort(port);
}

UpdClocksCmd::~UpdClocksCmd()
{
  delete m_event;
}

void UpdClocksCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdClocksCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void UpdClocksCmd::complete(CacheBuffer& cache)
{
  RSPUpdclocksEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  ack.handle = (uint32)this; // opaque pointer used to refer to the subscription

  ack.clocks().resize(m_event->tdmask.count());
  
  int result_td = 0;
  for (int cache_td = 0; cache_td < GET_CONFIG("RS.N_TDBOARDS", i); cache_td++)
  {
    if (m_event->tdmask[cache_td])
    {
      ack.clocks()(result_td) = cache.getClocks()()(cache_td);
      
      result_td++;
    }
  }

  // only send ack if clock setting has been modified
  if (cache.getClocks().getModified()) getPort()->send(ack);
}

const Timestamp& UpdClocksCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdClocksCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdClocksCmd::validate() const
{
  return (m_event->tdmask.count() <= (unsigned int)GET_CONFIG("RS.N_TDBOARDS", i));
}
