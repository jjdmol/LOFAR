//#  UpdXCStatsCmd.cc: implementation of the UpdXCStatsCmd class
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

#include "RSP_Protocol.ph"
#include "UpdXCStatsCmd.h"

#include <PSAccess.h>
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;
using namespace RTC;

UpdXCStatsCmd::UpdXCStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSubxcstatsEvent(event);

  setOperation(oper);
  setPeriod(m_event->period);
  setPort(port);
}

UpdXCStatsCmd::~UpdXCStatsCmd()
{
  delete m_event;
}

void UpdXCStatsCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdXCStatsCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void UpdXCStatsCmd::complete(CacheBuffer& cache)
{
  LOG_INFO_STR("UpdXCStatsCmd completed at time=" << getTimestamp());

  RSPUpdxcstatsEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  ack.handle = (uint32)this; // opaque pointer used to refer to the subscription

  TinyVector<int, 4> s = cache.getXCStats()().shape();
  s(2) = (m_event->rcumask.count()+1) / 2;
  ack.stats().resize(s);
  
  int result_rcu = 0;
  for (int cache_rcu = 0;
       cache_rcu < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
       cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      Range all = Range::all();
      ack.stats()(result_rcu % MEPHeader::N_POL, all, result_rcu / MEPHeader::N_POL, all) 
	= cache.getXCStats()()(cache_rcu % MEPHeader::N_POL, all, cache_rcu / MEPHeader::N_POL, all);
      
      result_rcu++;
    }
  }

  getPort()->send(ack);
}

const Timestamp& UpdXCStatsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdXCStatsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdXCStatsCmd::validate() const
{
  return ((int)m_event->rcumask.count() <= GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL);
}
