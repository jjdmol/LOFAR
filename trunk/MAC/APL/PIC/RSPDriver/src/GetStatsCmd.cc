//#  GetStatsCmd.cc: implementation of the GetStatsCmd class
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

#include <APLConfig.h>
#include "RSP_Protocol.ph"
#include "GetStatsCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

GetStatsCmd::GetStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPGetstatsEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

GetStatsCmd::~GetStatsCmd()
{
  delete m_event;
}

void GetStatsCmd::ack(CacheBuffer& cache)
{
  RSPGetstatsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status    = SUCCESS;
  
  if (m_event->type <= Statistics::SUBBAND_POWER)
  {
    ack.stats().resize(1, m_event->rcumask.count(),
		       cache.getSubbandStats()().extent(thirdDim));
  }
  else
  {
    ack.stats().resize(1, m_event->rcumask.count(),
		       cache.getBeamletStats()().extent(thirdDim));
  }
  
  int result_rcu = 0;
  for (int cache_rcu = 0; cache_rcu < GET_CONFIG("N_BLPS", i) * N_POL; cache_rcu++)
  {
    if (m_event->rcumask[result_rcu])
    {
      if (result_rcu < GET_CONFIG("N_BLPS", i) * N_POL)
      {
	if (m_event->type <= Statistics::SUBBAND_POWER)
	{
	  ack.stats()(0, result_rcu, Range::all())
	    = cache.getSubbandStats()()(m_event->type, cache_rcu, Range::all());
	}
	else
	{
	  ack.stats()(0, result_rcu, Range::all())
	    = cache.getBeamletStats()()(m_event->type - Statistics::BEAMLET_MEAN,
					cache_rcu, Range::all());
	}
      }
      else
      {
	LOG_WARN(formatString("invalid RCU index %d, there are only %d RCU's",
			      result_rcu, GET_CONFIG("N_RCUS", i)));
      }
      
      result_rcu++;
    }
  }
  
  getPort()->send(ack);
}

void GetStatsCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void GetStatsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetStatsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetStatsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetStatsCmd::validate() const
{
  return ((m_event->rcumask.count()
	  <= (unsigned int)GET_CONFIG("N_BLPS", i) * N_POL)
	  && (m_event->type < Statistics::N_STAT_TYPES));
}

bool GetStatsCmd::readFromCache() const
{
  return m_event->cache;
}

void GetStatsCmd::ack_fail()
{
  RSPGetstatsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = FAILURE;
  ack.stats().resize(0, 0, 0);

  getPort()->send(ack);
}
