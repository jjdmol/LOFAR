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

#include <APLConfig.h>
#include "RSP_Protocol.ph"
#include "SetSubbandsCmd.h"

#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace blitz;

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
  for (int cache_blp = 0;
       cache_blp < GET_CONFIG("N_RSPBOARDS", i) * GET_CONFIG("N_BLPS", i);
       cache_blp++)
  {
    if (m_event->blpmask[cache_blp])
    {
      if (cache_blp < GET_CONFIG("N_RSPBOARDS", i) * GET_CONFIG("N_BLPS", i))
      {
//	cache.getSubbandSelection()()(cache_blp, Range(0, N_BEAMLETS * 2 - 1))
//	  = m_event->subbands()(0, Range(0, N_BEAMLETS * 2 - 1));

	cache.getSubbandSelection()()(cache_blp, Range::all()) = 0;
	cache.getSubbandSelection()()(cache_blp, Range(0, m_event->subbands().extent(secondDim) - 1))
	  = m_event->subbands()(0, Range(0, m_event->subbands().extent(secondDim) - 1));

	LOG_DEBUG_STR("m_event->subbands() = " << m_event->subbands());
      }
      else
      {
	LOG_WARN(formatString("invalid BLP index %d, there are only %d BLP's",
			      cache_blp, GET_CONFIG("N_RSPBOARDS", i) * GET_CONFIG("N_BLPS", i)));
      }
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
  return ((m_event->blpmask.count() <= (unsigned int)GET_CONFIG("N_RSPBOARDS", i) * GET_CONFIG("N_BLPS", i))
	  && (2 == m_event->subbands().dimensions())
	  && (1 == m_event->subbands().extent(firstDim)));
}
