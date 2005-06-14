//#  GetXCStatsCmd.cc: implementation of the GetXCStatsCmd class
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
#include "GetXCStatsCmd.h"

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

GetXCStatsCmd::GetXCStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPGetxcstatsEvent(event);

  m_n_devices = GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

GetXCStatsCmd::~GetXCStatsCmd()
{
  delete m_event;
}

void GetXCStatsCmd::ack(CacheBuffer& cache)
{
  RSPGetxcstatsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;

  TinyVector<int, 4> s = cache.getXCStats()().shape();
  s(2) = m_event->rcumask.count() / 2;
  ack.stats().resize(s);
  
  int result_device = 0;
  for (unsigned int cache_device = 0; cache_device < m_n_devices; cache_device++)
  {
    if (m_event->rcumask[cache_device])
    {
      Range all = Range::all();

      ack.stats()(result_device % MEPHeader::N_POL, all, result_device / MEPHeader::N_POL, all) 
	= cache.getXCStats()()(cache_device % MEPHeader::N_POL, all, cache_device / MEPHeader::N_POL, all);
      
      result_device++;
    }
  }

  getPort()->send(ack);
}

void GetXCStatsCmd::apply(CacheBuffer& /*cache*/)
{
  // no-op
}

void GetXCStatsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetXCStatsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetXCStatsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetXCStatsCmd::validate() const
{
  return (m_event->rcumask.count() <= m_n_devices);
}

bool GetXCStatsCmd::readFromCache() const
{
  return m_event->cache;
}

void GetXCStatsCmd::ack_fail()
{
  RSPGetxcstatsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = FAILURE;
  ack.stats().resize(0, 0, 0, 0);

  getPort()->send(ack);
}
