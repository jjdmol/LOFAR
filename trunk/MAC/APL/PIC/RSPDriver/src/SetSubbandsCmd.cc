//#  SetSubbandsCmd.cc: implementation of the SetSubbandsCmd class
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

#include "RSP_Protocol.ph"
#include "SetSubbandsCmd.h"

#include <PSAccess.h>
#include <blitz/array.h>

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetSubbandsCmd::SetSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
  m_event = new RSPSetsubbandsEvent(event);

  setOperation(oper);
  setPeriod(0);
  setPort(port);
}

SetSubbandsCmd::~SetSubbandsCmd()
{
  delete m_event;
}

void SetSubbandsCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetsubbandsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = SUCCESS;
  
  getPort()->send(ack);
}

void SetSubbandsCmd::apply(CacheBuffer& cache)
{
  for (int cache_rcu = 0; cache_rcu < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL; cache_rcu++)
  {
    if (m_event->rcumask[cache_rcu])
    {
      cache.getSubbandSelection()()(cache_rcu, Range::all()) = 0;
      cache.getSubbandSelection()()(cache_rcu, Range(0, m_event->subbands().extent(secondDim) - 1))
	= m_event->subbands()(0, Range(0, m_event->subbands().extent(secondDim) - 1)) * 2 + (cache_rcu % MEPHeader::N_POL);
      
      LOG_DEBUG_STR("m_event->subbands() = " << m_event->subbands());
    }
  }
}

void SetSubbandsCmd::complete(CacheBuffer& /*cache*/)
{
  LOG_INFO_STR("SetSubbandsCmd completed at time=" << getTimestamp());
}

const Timestamp& SetSubbandsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetSubbandsCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
	  && (2 == m_event->subbands().dimensions())
	  && (1 == m_event->subbands().extent(firstDim))
	  && (m_event->subbands().extent(secondDim) <= MEPHeader::N_XBLETS)); // should be N_BEAMLETS once we have a separate SetXCSubbandCmd
}
