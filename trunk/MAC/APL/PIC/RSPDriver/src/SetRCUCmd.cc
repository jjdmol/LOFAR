//#  SetRCUCmd.cc: implementation of the SetRCUCmd class
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
#include "RSPConfig.h"
#include "SetRCUCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

SetRCUCmd::SetRCUCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSetrcuEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

SetRCUCmd::~SetRCUCmd()
{
  delete m_event;
}

void SetRCUCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetrcuackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  
  getPort()->send(ack);
}

void SetRCUCmd::apply(CacheBuffer& cache)
{
  for (int cache_rcu = 0; cache_rcu < GET_CONFIG("N_RCU", i); cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      if (cache_rcu < GET_CONFIG("N_RCU", i))
      {
	cache.getRCUSettings()()(cache_rcu) = m_event->settings()(0);
      }
      else
      {
	LOG_WARN(formatString("invalid RCU index %d, there are only %d RCU's",
			      cache_rcu, GET_CONFIG("N_RCU", i)));
      }
    }
    
  }
}

void SetRCUCmd::complete(CacheBuffer& /*cache*/)
{
  LOG_INFO_STR("SetRCUCmd completed at time=" << getTimestamp());
}

const Timestamp& SetRCUCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetRCUCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetRCUCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)GET_CONFIG("N_RCU", i))
	  && (1 == m_event->settings().dimensions())
	  && (1 == m_event->settings().extent(firstDim)));
}
