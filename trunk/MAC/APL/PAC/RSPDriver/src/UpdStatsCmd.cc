//#  UpdStatsCmd.cc: implementation of the UpdStatsCmd class
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
#include "UpdStatsCmd.h"

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

UpdStatsCmd::UpdStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSubstatsEvent(event);

  m_n_devices = ((m_event->type <= Statistics::SUBBAND_POWER)
		 ? GET_CONFIG("RS.N_BLPS", i) : 1)
    * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL;

  setOperation(oper);
  setPeriod(m_event->period);
  setPort(port);
}

UpdStatsCmd::~UpdStatsCmd()
{
  delete m_event;
}

void UpdStatsCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdStatsCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void UpdStatsCmd::complete(CacheBuffer& cache)
{
  RSPUpdstatsEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  ack.handle = (uint32)this; // opaque pointer used to refer to the subscription

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
  
  int result_device = 0;
  for (unsigned int cache_device = 0; cache_device < m_n_devices; cache_device++)
  {
    if (m_event->rcumask[cache_device])
    {
      if (m_event->type <= Statistics::SUBBAND_POWER)
      {
	ack.stats()(0, result_device, Range::all())
	  = cache.getSubbandStats()()(m_event->type,
				      cache_device, Range::all());
      }
      else
      {
	ack.stats()(0, result_device, Range::all())
	  = cache.getBeamletStats()()(m_event->type - Statistics::BEAMLET_MEAN,
				      cache_device, Range::all());
      }

      result_device++;
    }
  }

  getPort()->send(ack);
}

const Timestamp& UpdStatsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void UpdStatsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool UpdStatsCmd::validate() const
{
  return ((m_event->rcumask.count() <= m_n_devices)
	  && (m_event->type < Statistics::N_STAT_TYPES));
}
