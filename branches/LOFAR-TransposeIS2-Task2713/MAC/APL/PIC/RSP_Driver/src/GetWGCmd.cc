//#  GetWGCmd.cc: implementation of the GetWGCmd class
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
#include "GetWGCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetWGCmd::GetWGCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetWG", port, oper)
{
  m_event = new RSPGetwgEvent(event);
}

GetWGCmd::~GetWGCmd()
{
  delete m_event;
}

void GetWGCmd::ack(CacheBuffer& cache)
{
  RSPGetwgackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;

  ack.settings().resize(m_event->rcumask.count());
  
  int result_rcu = 0;
  for (int cache_rcu = 0;
       cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      if (cache_rcu < StationSettings::instance()->nrRcus())
      {
	ack.settings()(result_rcu) = cache.getWGSettings()()(cache_rcu);
      }
      else
      {
	LOG_WARN(formatString("invalid RCU index %d, there are only %d RCU's", cache_rcu, 
		StationSettings::instance()->nrRcus()));
      }
      
      result_rcu++;
    }
  }
  
  getPort()->send(ack);
}

void GetWGCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  /* intentionally left empty */
}

void GetWGCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetWGCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetWGCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetWGCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus()));
}

bool GetWGCmd::readFromCache() const
{
  return m_event->cache;
}
